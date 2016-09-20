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
        
        return 0;
}

// Assumes string of exact form:
// "nodeID T1 H1 T2 H2 T3 H3 T4 H4 R G B V\n" where nodeID is an integer, all T and H values are floating point, R,G and B are integers
// and V is floating point. T1/H1 are at 1m, T2/H2 are at 20cm, T3/H3 are at ground level, T4/H4 are underground, R,G and B are 
// values from light sensor and V is the battery voltage. 
//
// Example: "19 21.1 46.7 23.2 45.0 25.7 42.0 19.3 96.2 1205 4509 6399 3.89\n"
//
convertToXML(time_t timestamp, char* buffer)
{
	char* tempbuf;
	char* intermedbuf;
	int i = 0;
	int ivalue = 0;
	float fvalue = 0.0;
	char* ptr;
	
	strcpy(tempbuf, buffer);
	
	ptr = tempbuf[0];
	
	for (i = 0; i < sizeof(buffer); i++)
	{
		buffer[i] = 0;
	}
	
	sprintf(intermedbuf, "<sensor_reading>\n");
	strcat(buffer, intermedbuf);
	ivalue = atoi(ptr);
	sprintf(intermedbuf, "\t<nodeID> %d <\nodeID>\n", ivalue)
	strcat(buffer, intermedbuf);
	sprintf(intermedbuf, "\t<top>\n")
	strcat(buffer, intermedbuf);
	while (' ' != ptr++);
	fvalue = atof(ptr);
	sprintf(intermedbuf, "\t\t<Height> 1000mm <\Height>\n", fvalue)
	strcat(buffer, intermedbuf);
	sprintf(intermedbuf, "\t\t<Temperature> %f <\Temperature>\n", fvalue)
	strcat(buffer, intermedbuf);
	while (' ' != ptr++);
	fvalue = atof(ptr);
	sprintf(intermedbuf, "\t\t<Humidity> %f <\Humidity>\n", fvalue)
	strcat(buffer, intermedbuf);
	sprintf(intermedbuf, "\t<\top>\n")
	strcat(buffer, intermedbuf);
	sprintf(intermedbuf, "\t<middle>\n")
	strcat(buffer, intermedbuf);
	while (' ' != ptr++);
	fvalue = atof(ptr);
	sprintf(intermedbuf, "\t\t<Height> 200mm <\Height>\n", fvalue)
	strcat(buffer, intermedbuf);
	sprintf(intermedbuf, "\t\t<Temperature> %f <\Temperature>\n", fvalue)
	strcat(buffer, intermedbuf);
	while (' ' != ptr++);
	fvalue = atof(ptr);
	sprintf(intermedbuf, "\t\t<Humidity> %f <\Humidity>\n", fvalue)
	strcat(buffer, intermedbuf);
	sprintf(intermedbuf, "\t<\middle>\n")
	strcat(buffer, intermedbuf);
	sprintf(intermedbuf, "\t<bottom>\n")
	strcat(buffer, intermedbuf);
	while (' ' != ptr++);
	fvalue = atof(ptr);
	sprintf(intermedbuf, "\t\t<Height> 0mm <\Height>\n", fvalue)
	strcat(buffer, intermedbuf);
	sprintf(intermedbuf, "\t\t<Temperature> %f <\Temperature>\n", fvalue)
	strcat(buffer, intermedbuf);
	while (' ' != ptr++);
	fvalue = atof(ptr);
	sprintf(intermedbuf, "\t\t<Humidity> %f <\Humidity>\n", fvalue)
	strcat(buffer, intermedbuf);
	sprintf(intermedbuf, "\t<\bottom>\n")
	strcat(buffer, intermedbuf);
	sprintf(intermedbuf, "\t<ground>\n")
	strcat(buffer, intermedbuf);
	while (' ' != ptr++);
	fvalue = atof(ptr);
	sprintf(intermedbuf, "\t\t<Height> -50mm <\Height>\n", fvalue)
	strcat(buffer, intermedbuf);
	sprintf(intermedbuf, "\t\t<Temperature> %f <\Temperature>\n", fvalue)
	strcat(buffer, intermedbuf);
	while (' ' != ptr++);
	fvalue = atof(ptr);
	sprintf(intermedbuf, "\t\t<Humidity> %f <\Humidity>\n", fvalue)
	strcat(buffer, intermedbuf);
	sprintf(intermedbuf, "\t<\ground>\n")
	strcat(buffer, intermedbuf);
	sprintf(intermedbuf, "\t<light>\n")
	strcat(buffer, intermedbuf);
	while (' ' != ptr++);
	ivalue = atoi(ptr);
	sprintf(intermedbuf, "\t\t<Red> %d <\Red>\n", ivalue)
	strcat(buffer, intermedbuf);
	while (' ' != ptr++);
	ivalue = atoi(ptr);
	sprintf(intermedbuf, "\t\t<Green> %d <\Green>\n", ivalue)
	strcat(buffer, intermedbuf);
	while (' ' != ptr++);
	ivalue = atoi(ptr);
	sprintf(intermedbuf, "\t\t<Blue> %d <\Blue>\n", ivalue)
	strcat(buffer, intermedbuf);
	sprintf(intermedbuf, "\t<\light>\n")
	strcat(buffer, intermedbuf);
	
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
	time_t curr_time = time(NULL); 
	char recvbuf [4096];
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
		recvbuf[i] = 0;
	}

	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, "http://users.aber.ac.uk/mjn/recorddata.php");

	while (1)
	{
		while(buf[0] != '\n') // read a line of data (data packet received from a node) from the moteino
		{
			n = read (tty_fd, buf, 1); 
			if (n > 0)
			{
				recvbuf[pos++] = buf[0];
			}
			n = 0;
		}
		buf[0] = 0;
		total += pos;
		if (total > 10) // if less than 10 characters then it's probably rubbish
		{
			curr_time = time(NULL);  
			//convertToXML(curr_time, recvbuf);
			sprintf(finalbuf, "data= %s %s", asctime(gmtime(&curr_time)), recvbuf); // add time stamp to data packet
			/* Now specify the POST data */
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, finalbuf);
			printf(finalbuf);

			/* Perform the request, res will get the return code */
			res = curl_easy_perform(curl);
			total = 0;
			for (i = 0; i < sizeof(finalbuf); i++) // clear out buffers
			{
				recvbuf[i] = 0;
				finalbuf[i] = 0;
			}
			pos = 0;
		}
	}
}
