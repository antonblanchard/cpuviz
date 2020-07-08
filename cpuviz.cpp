#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <iostream>

#include "cpuviz.h"

#include <QPainter>
#include <QTimer>
#include <QApplication>

/* Size of each CPU rectangle */
#define RECWIDTH 25
#define RECHEIGHT 25

/* Size of border around the CPU rectangles */
#define BORDER 20

/* Gap between CPU rectangles */
#define GAP 5

#define FPS 24

#define STATBUFSIZE (128*1024)

static unsigned int round_closest(unsigned int dividend, unsigned int divisor)
{
	return (dividend + (divisor / 2)) / divisor;
}

Widget::Widget(unsigned long xlenIn, unsigned long ylenIn, QWidget *parent) :
	QWidget(parent)
{
	statF = fopen("/proc/stat", "r");
	assert(statF);

	statbuf = (char *)malloc(STATBUFSIZE);
	assert(statbuf);
	memset(statbuf, 0, STATBUFSIZE);
	setbuffer(statF, statbuf, STATBUFSIZE);

	QTimer *timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, QOverload<>::of(&Widget::update));
	timer->start(round_closest(1000, FPS));

	for (unsigned long i = 0; i < MAX_CPUS; i++)
		cpus_busy[i] = 0;

	xlen = xlenIn;
	ylen = ylenIn;
}

Widget::~Widget()
{
	fclose(statF);
	free(statbuf);
}

void Widget::parseStat(void)
{
	rewind(statF);

	while (!feof(statF)) {
		char line[LINE_MAX];
		unsigned int cpu;
		int64_t user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
		int scanned;

		fgets(line, sizeof(line), statF);

		scanned = sscanf(line, "cpu%u"
				 " %" SCNu64
				 " %" SCNu64
				 " %" SCNu64
				 " %" SCNu64
				 " %" SCNu64
				 " %" SCNu64
				 " %" SCNu64
				 " %" SCNu64
				 " %" SCNu64
				 " %" SCNu64
				 , &cpu, &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guest_nice);

		if (scanned == 11) {
			uint64_t total, busy_total, busy_diff, total_diff;

			assert(cpu < MAX_CPUS);

			busy_total = user+nice+system+irq+softirq+steal+guest+guest_nice;
			total = idle+iowait+busy_total;

			busy_diff = busy_total - cpus_prev_busy_total[cpu];
			total_diff = total - cpus_prev_total[cpu];

			cpus_busy[cpu] = 255.0*busy_diff/total_diff;

			cpus_prev_busy_total[cpu] = busy_total;
			cpus_prev_total[cpu] = total;
		}
	}
}

void Widget::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	int xpos=BORDER;
	int ypos=BORDER;

	parseStat();

	for (unsigned long x=0; x < xlen; x++) {
		ypos = BORDER;

		for (unsigned long y=0; y < ylen; y++) {
			QColor color = QColor::fromCmyk(0, 0, 0, cpus_busy[x+y*xlen], 255);
			p.fillRect(xpos, ypos, RECWIDTH, RECHEIGHT, color);
			ypos += RECHEIGHT+GAP;
		}

		xpos += RECWIDTH+GAP;
	}

	resize(BORDER*2+xlen*(RECWIDTH+GAP), BORDER*2+ylen*(RECHEIGHT+GAP));
}

static void usage(void)
{
	std::cerr << "Usage: cpuviz [width] [ylen]" << std::endl;
	exit(1);
}

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	QStringList args = app.arguments();
	if (args.count() != 3)
		usage();

	bool ok;
	unsigned long xlen = args[1].toLong(&ok, 10);
	if (ok == 0)
		usage();
	unsigned long ylen = args[2].toLong(&ok, 10);
	if (ok == 0)
		usage();

	Widget widget(xlen, ylen);
	widget.setWindowTitle("cpuviz");
	widget.show();

	return app.exec();
}
