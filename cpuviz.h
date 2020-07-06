#ifndef WIDGET_H
#define WIDGET_H

#include <stdio.h>

#include <QWidget>

#define MAX_CPUS 2048

class Widget : public QWidget
{
    Q_OBJECT

    public:
        explicit Widget(unsigned long xlen, unsigned long ylen, QWidget *parent = 0);
        ~Widget();

    protected:
        void paintEvent(QPaintEvent *event) override;

    private:
        void parseStat(void);
        FILE *statF;
        uint64_t cpus_prev_busy_total[MAX_CPUS];
        uint64_t cpus_prev_total[MAX_CPUS];
        unsigned short cpus_busy[MAX_CPUS];
	unsigned long xlen;
	unsigned long ylen;
};

#endif // WIDGET_H
