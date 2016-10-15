#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <curl/curl.h>
#include <errno.h>

int main()
{
	int status = 0;
	char i2cbuf[3];
	double cputemp;
	FILE *fd;
	CURL *curl;
	CURLcode res;
	time_t result = time(NULL);
	char webbuf [4096];
	curl = curl_easy_init();

	fd = fopen("/sys/devices/virtual/thermal/thermal_zone0/temp", "r");

	fscanf(fd, "%lf", &cputemp);

	cputemp /= 1000;

	int i2cdev = wiringPiI2CSetup (0x3a);
	if (i2cdev < 0)
	{
		printf("setting up ADM1911 failed!\n Exiting...\n");
		return(-1);
	}

	status = wiringPiI2CWrite(i2cdev, 0x00); // Ensure that all bits are set to zero (select correct voltage range etc)
	// See ADM1191 datasheet for details
	// Check return status
	if (status < 0)
	{
		printf("setting up ADM1911 failed!\n Exiting...\n");
		return(-1);
	}

	status = wiringPiI2CWrite(i2cdev, 0x02); // Trigger voltage reading
	if (status < 0)
	{
		printf("Reading voltage failed!\n Exiting...\n");
		return(-1);
	}
	usleep(100000);			// Wait a while for conversion to complete
	read (i2cdev, i2cbuf, 2);		// Read the two bytes of data
	float voltage = (float) ((26.52/4096) * (float) ((i2cbuf[0]*16) + (i2cbuf[1]/16)));
	// Convert to voltage (see ADM1191 datasheet for details)

	status = wiringPiI2CWrite(i2cdev, 0x08); // Trigger current reading
	if (status < 0)
	{
		printf("Reading current failed!\n Exiting...\n");
		return(-1);
	}
	usleep(100000);			// Wait a while for conversion to complete
	read (i2cdev, i2cbuf, 2);		// Read the two bytes of data
	float current = (float) (((105.84/4096) * (float) ((i2cbuf[0]*16) + (i2cbuf[1]/16)))/0.1);
	// Convert to current (see ADM1191 datasheet for details)
	sprintf(webbuf, "power= Voltage: %.2lf V Current: %.2lf mA CPU Temp: %.2lf C at %s\n", voltage, current, cputemp, asctime(gmtime(&result)));
	printf(webbuf);
	if(curl) {
		curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_URL, "http://users.aber.ac.uk/mjn/zambiapowerbase1.php");
		/* First set the URL that is about to receive our POST. This URL can
		   just as well be a https:// URL if that is what should receive the
		   data. */
		/* Now specify the POST data */
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, webbuf);

		/* Perform the request, res will get the return code */
		res = curl_easy_perform(curl);
	}
	return res;
}
