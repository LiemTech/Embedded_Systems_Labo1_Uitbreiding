#include "labo1.h"
#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <gpiod.h>

struct bcm2835_peripheral gpio = {GPIO_BASE};
struct bcm2835_peripheral bsc0 = {BSC0_BASE};

int map_peripheral(struct bcm2835_peripheral *p)
{
   if ((p->mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
      printf("Failed to open /dev/mem, try checking permissions.\n");
      return -1;
   }

   p->map = mmap(
      NULL,
      BLOCK_SIZE,
      PROT_READ|PROT_WRITE,
      MAP_SHARED,
      p->mem_fd,
      p->addr_p
   );

   if (p->map == MAP_FAILED) {
        perror("mmap");
        return -1;
   }

   p->addr = (volatile unsigned int *)p->map;

   return 0;
}

void unmap_peripheral(struct bcm2835_peripheral *p) {

    munmap(p->map, BLOCK_SIZE);
    close(p->mem_fd);
}

void dump_bsc_status() {

    unsigned int s = BSC0_S;

    printf("BSC0_S: ERR=%d  RXF=%d  TXE=%d  RXD=%d  TXD=%d  RXR=%d  TXW=%d  DONE=%d  TA=%d\n",
        (s & BSC_S_ERR) != 0,
        (s & BSC_S_RXF) != 0,
        (s & BSC_S_TXE) != 0,
        (s & BSC_S_RXD) != 0,
        (s & BSC_S_TXD) != 0,
        (s & BSC_S_RXR) != 0,
        (s & BSC_S_TXW) != 0,
        (s & BSC_S_DONE) != 0,
        (s & BSC_S_TA) != 0 );
}

void wait_i2c_done() {
        int timeout = 50;
        while((!((BSC0_S) & BSC_S_DONE)) && --timeout) {
            usleep(1000);
        }
        if(timeout == 0)
            printf("wait_i2c_done() timeout. Something went wrong.\n");
}

void i2c_init()
{
    INP_GPIO(0);
    SET_GPIO_ALT(0, 0);
    INP_GPIO(1);
    SET_GPIO_ALT(1, 0);
} 

int SetProgramPriority(int priorityLevel)
{
    struct sched_param sched;

    memset (&sched, 0, sizeof(sched));

    if (priorityLevel > sched_get_priority_max (SCHED_RR))
        priorityLevel = sched_get_priority_max (SCHED_RR);

    sched.sched_priority = priorityLevel;

    return sched_setscheduler (0, SCHED_RR, &sched);
}

int main(int argc, char **argv)
{

  if (map_peripheral(&gpio) == -1)
    {
        printf("Failed to map the physical GPIO registers into the virtual memory space.\n");
        return -1;
    }

    INP_GPIO(17);
    INP_GPIO(22);
    INP_GPIO(27);

    const char *chipname = "gpiochip0";
    struct gpiod_chip *chip;
    struct gpiod_line *button1;
    struct gpiod_line *button2;
    struct gpiod_line *button3;

    chip = gpiod_chip_open_by_name(chipname);

    button1 = gpiod_chip_get_line(chip, 17);
    button2 = gpiod_chip_get_line(chip, 22);
    button3 = gpiod_chip_get_line(chip, 27);

    bool button1A = false;
    bool button1B = false;
    bool button2A = false;
    bool button2B = false;
    bool button3A = false;
    bool button3B = false;

    while (1)
    {
        val1 = gpiod_line_get_value(button1);
        val2 = gpiod_line_get_value(button2);
        val3 = gpiod_line_get_value(button3);

        MYSQL *con = mysql_init(NULL);

        if (con == NULL)
        {
            fprintf(stderr, "%s\n", mysql_error(con));
            exit(1);
        }

        if (mysql_real_connect(con, "localhost", "liem", "liem123", "labo1", 0, NULL, 0) == NULL)
        {
            fprintf(stderr, "%s\n", mysql_error(con));
            mysql_close(con);
            exit(1);
        }

        if (val1 == 1)
        {
            button1A = true;

            if(button1B == false)
            {
                mysql_query(con, "INSERT INTO inputs (input, state) VALUES ('input 1', 'pressed');");
                printf("Button 1 has been pressed.\n");
                button1B = true;
            }
            sleep(1);
        }
        if (val2 == 1)
        {
            button2A = true;

            if(button2B == false)
            {
                mysql_query(con, "INSERT INTO inputs (input, state) VALUES ('input 2', 'pressed');");
                printf("Button 2 has been pressed.\n");
                button2B = true;
            }
            sleep(1);
        }
        if (val3 == 1)
        {
            button3A = true;

            if(button3B == false)
            {
                mysql_query(con, "INSERT INTO inputs (input, state) VALUES ('input 3', 'pressed');");
                printf("Button 3 has been pressed.\n");
                button3B = true;
            }
            sleep(1);
        }
        else
        {
            button1A = false;
            button2A = false;
            button3A = false;

            if(button1B == true)
            {
                mysql_query(con, "INSERT INTO inputs (input, state) VALUES ('input 1', 'released');");
                printf("Button 1 has been released.\n");
                button1B = false;
            }

            if(button2B == true)
            {
                mysql_query(con, "INSERT INTO inputs (input, state) VALUES ('input 2', 'released');");
                printf("Button 2 has been released.\n");
                button2B = false;
            }

            if(button3B == true)
            {
                mysql_query(con, "INSERT INTO inputs (input, state) VALUES ('input 3', 'released');");
                printf("Button 3 has been released.\n");
                button3B = false;
            }
            sleep(1);
        }
          mysql_close(con);
    }

    return 0;

}