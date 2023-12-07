#include <gpiod.h>
#include <error.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>

#define DEV_I2C_BUS   10
#define DEV_I2C_ADDR  0x20
#define LVD_HOOK_EXEC "echo LVD trigger callback script"  //custom callback script

#define msleep(x) usleep((x)*1000)

extern int errno;
unsigned int m_lvd_gpio = 0;

void str_CR_LF_remove(char *param_str)
{
    int size, i;
    
    size = strlen(param_str);

    /* start check from ending char.*/
    for (i = size - 1; i > 0; --i)
    {
        if ('\r' == param_str[i] 
         || '\n' == param_str[i])
        {
            param_str[i] = '\0';
        }
        else
        {
            break;
        }
    }
}

int check_shell_status(int rv)
{
    /* no child pricess execute.*/
    if (-1 == rv)
    {
        return -1;
    }

    /* check child process normal exit.*/
    if (!WIFEXITED(rv))
    {
        return -1;
    }

    /* check shell execute correct.*/
    if (0 != WEXITSTATUS(rv))  
    {
        return -1;
    }  
    
     return 0;  
}

int utils_system_ex(const char *cmd, char *recv, uint16_t max_size)
{
    int ret = -1, rv, sizet = 0;
    FILE *fp = NULL;
	uint32_t cnt = 0;
	
	/* we use pipe to execute this cmd.*/
	fp = popen(cmd, "r");
	if (NULL == fp)
    {
        return -1;
    }
    
    if (NULL != recv)
    {
        do
        {
            sizet = fread(recv, sizeof(char), max_size, fp);
            if (++cnt > 10)
            {
                sizet = 0;
                goto _per_end;
            }
        }
        while(sizet <= 0 && EINTR == errno);
        
        sizet = (sizet >= max_size) ? max_size - 1 : sizet;
        recv[sizet] = '\0';
    }
    
_per_end:
    rv = pclose(fp);
    if (-1 == rv && ECHILD == errno)
    {
        if (sizet > 0)
        {
            ret = sizet;
        }
    }
    else
    {
        ret = check_shell_status(rv);
        if (0 == ret)
        {
            ret = sizet;
        }
    }
    
	return ret;
}

void trig_state_hook(unsigned int offset)
{
  char command[100];

  if(offset == m_lvd_gpio)
  {
    strcpy(command, LVD_HOOK_EXEC);
    printf("Low voltage\n");

    system(command);
  }
}

int main(int argc, char *argv[])
{
    char cmd[256] = {0};
    char gpiochip_n[16] = {0};
    char lvd_gpio[16] = {0};
    char dev_gpiochio[32] = {0};
    unsigned int offsets[1];
    int values[1] = {-1};
    int err = 0;
    struct timespec *timeout=NULL;
    struct gpiod_chip *chip;
    struct gpiod_line_bulk lines;
    struct gpiod_line_bulk events;

    while(1)
    {
      msleep(200);
      //Obtain LVD gpiochip number
      sprintf(cmd, "gpiodetect | grep %d-%04x | sed 's/gpiochip//' | awk '{print $1}'",DEV_I2C_BUS,DEV_I2C_ADDR);
      err = utils_system_ex(cmd, gpiochip_n, 16);
      if(err < 0)
      {
        continue;
      }

      str_CR_LF_remove(gpiochip_n);

      //Obtain LVD gpio pin number
      sprintf(cmd, "gpioinfo %s | grep 'LVD' | cut -d ':' -f 1 | awk '{print $2}'",gpiochip_n);
      err = utils_system_ex(cmd, lvd_gpio, 16);
      if(err < 0)
      {
        continue;
      }

      str_CR_LF_remove(lvd_gpio);

      break;
    }

    // use pin as input
    m_lvd_gpio = atoi(lvd_gpio);
    offsets[0] = m_lvd_gpio;

    while(1)
    {
      msleep(10);
      sprintf(dev_gpiochio, "/dev/gpiochip%s",gpiochip_n);
      
      chip = gpiod_chip_open(dev_gpiochio);
      if(!chip)
      {
        perror("gpiod_chip_open");
        err = -1;
        goto cleanup;
      }

      gpiod_line_bulk_init(&lines);
      err = gpiod_chip_get_lines(chip, offsets, 1, &lines);
      if(err)
      {
        perror("gpiod_chip_get_lines");
        goto cleanup;
      }

	  err = gpiod_line_request_bulk_falling_edge_events(&lines, "falling edge");
      if(err)
      {
        perror("gpiod_line_request_bulk_falling_edge_events");
        goto cleanup;
      }

      err = gpiod_line_event_wait_bulk(&lines, timeout, &events);
      if(err == -1)
      {
        perror("gpiod_line_event_wait_bulk");
        goto cleanup;
      }
      else if(err == 0)
      {
        fprintf(stderr, "wait timed out\n");
        goto cleanup;
      }

      err = gpiod_line_get_value_bulk(&events, values);
      if(err)
      {
        perror("gpiod_line_get_value_bulk");
        goto cleanup;
      }

      for(int i=0; i<gpiod_line_bulk_num_lines(&events); i++)
      {
        struct gpiod_line* line;
        line = gpiod_line_bulk_get_line(&events, i);
        if(!line)
        {
          fprintf(stderr, "unable to get line %d\n", i);
          continue;
        }

        unsigned int offset = gpiod_line_offset(line);
        trig_state_hook(offset);
      }

      cleanup:
        gpiod_line_release_bulk(&lines);
        gpiod_chip_close(chip);
    }

    return err;
}