#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <wiringPi.h>
#include <curl/curl.h>
#include <errno.h>

int tty_fd;

int open_moteino(void)
{
	struct termios tio;

	// Open the device
	//int tty_fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);
	tty_fd = open("/dev/ttyS0", O_RDWR | O_NOCTTY | O_NDELAY);

	// Check if the port didn't open correctly
	if (tty_fd==-1)
	{
		return -1;
	}

	tio.c_iflag = 0;
	tio.c_oflag = 0;
	tio.c_cflag = CS8 | CREAD | CLOCAL;
	tio.c_lflag = 0;
	tio.c_cc[VMIN] = 0;
	tio.c_cc[VTIME] = 0;

	if (cfsetospeed(&tio, B115200) || cfsetispeed(&tio, B115200))
	{
		return -1;
	}

	cfmakeraw(&tio);
	tcflush(tty_fd, TCIFLUSH);
	tcsetattr(tty_fd, TCSANOW, &tio);

}

main()
{
	int n = 0;
	int i = 0;
	int pos = 0;
	int total = 0;
	char buf[1];
	CURL *curl;
	CURLcode res;
	time_t result = time(NULL); 
	char webbuf [4096];
	char finalbuf [4096];
	curl = curl_easy_init();

	if (-1 == open_moteino())
	{
		printf("Failed to open Moteino!");
		return -1;
	}

	for (i = 0; i < sizeof(finalbuf); i++)
	{
		finalbuf[i] = 0;
		webbuf[i] = 0;
	}

	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, "http://users.aber.ac.uk/mjn/recorddata.php");

	while (1)
	{
		while(buf[0] != '\n')
		{
			n = read (tty_fd, buf, 1); 
			if (n > 0)
			{
				webbuf[pos++] = buf[0];
			}
			n = 0;
		}
		buf[0] = 0;
		total += pos;
		if (total > 10)
		{
			result = time(NULL); 
			sprintf(finalbuf, "data= %s %s", asctime(gmtime(&result)), webbuf);
			/* Now specify the POST data */
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, finalbuf);
			printf(finalbuf);

			/* Perform the request, res will get the return code */
			res = curl_easy_perform(curl);
			total = 0;
			for (i = 0; i < sizeof(finalbuf); i++)
			{
				webbuf[i] = 0;
				finalbuf[i] = 0;
			}
			pos = 0;
		}
	}
}
