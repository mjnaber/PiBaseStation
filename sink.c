#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
//#include <wiringPi.h>
#include <curl/curl.h>
#include <errno.h>
#include <stdio.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>

#define MY_ENCODING "ISO-8859-1"
#define MAXQUEUE 1048576

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

main()
{
	int n = 0;
	int i = 0;
	int pos = 0;
	int total = 0;
	char cbuf[1];
	char* ptr;
	CURL *curl;
	CURLcode res;
	time_t result = time(NULL); 
	char webbuf [4096];
	char finalbuf [4096];
	char queued [MAXQUEUE];// [1048576]; // Allocate 1M of RAM for buffering failed transmission attempts until network allows CURL to succeed
	curl = curl_easy_init();

	int rc;
	xmlTextWriterPtr writer;
	xmlBufferPtr buf;

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

	/* Create a new XML buffer, to which the XML document will be
	 * written */
	buf = xmlBufferCreate();
	if (buf == NULL) {
		printf("testXmlwriterMemory: Error creating the xml buffer\n");
		return -1;
	}

	writer = xmlNewTextWriterMemory(buf, 0);
	if (writer == NULL) {
		printf("testXmlwriterMemory: Error creating the xml writer\n");
		return -1;
	}

	/* Start the document with the xml default for the version,
	 * encoding ISO 8859-1 and the default for the standalone
	 * declaration. */
	xmlTextWriterSetIndent(writer, 1);


	while (1)
	{
		while(cbuf[0] != '\n')
		{
			n = read (tty_fd, cbuf, 1); 
			if (n > 0)
			{
				webbuf[pos++] = cbuf[0];
			}
			n = 0;
		}
		ptr = &webbuf[0];
		cbuf[0] = 0;
		total += pos;
		if (total > 10)
		{
			result = time(NULL); 
			sprintf(finalbuf, "data= %s %s", asctime(gmtime(&result)), webbuf);
			rc = xmlTextWriterStartDocument(writer, NULL, MY_ENCODING, NULL);

			/* Start an element named "EXAMPLE". Since thist is the first
			 * element, this will be the root element of the document. */
			rc += xmlTextWriterStartElement(writer, BAD_CAST "data_point");
			rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "node_ID", "%d", atoi(ptr));
			ptr = strchr(ptr, ' '); if (ptr == NULL) goto wipe; 
			rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "message_type", "%d", atoi(ptr));
			rc += xmlTextWriterStartElement(writer, BAD_CAST "one_metre");
			ptr = strchr(ptr, ' '); if (ptr == NULL) goto wipe; 
			rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "temperature", "%05.2f", atof(ptr));
			ptr = strchr(ptr, ' '); if (ptr == NULL) goto wipe; 
			rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "humidity", "%05.2f", atof(ptr));
			rc += xmlTextWriterEndElement(writer);
			rc += xmlTextWriterStartElement(writer, BAD_CAST "twenty_centimetres");
			ptr = strchr(ptr, ' '); if (ptr == NULL) goto wipe; 
			rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "temperature", "%05.2f", atof(ptr));
			ptr = strchr(ptr, ' '); if (ptr == NULL) goto wipe; 
			rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "humidity", "%05.2f", atof(ptr));
			rc += xmlTextWriterEndElement(writer);
			rc += xmlTextWriterStartElement(writer, BAD_CAST "ground_level");
			ptr = strchr(ptr, ' '); if (ptr == NULL) goto wipe; 
			rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "temperature", "%05.2f", atof(ptr));
			ptr = strchr(ptr, ' '); if (ptr == NULL) goto wipe; 
			rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "humidity", "%05.2f", atof(ptr));
			rc += xmlTextWriterEndElement(writer);
			rc += xmlTextWriterStartElement(writer, BAD_CAST "soil");
			ptr = strchr(ptr, ' '); if (ptr == NULL) goto wipe; 
			rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "temperature", "%05.2f", atof(ptr));
			ptr = strchr(ptr, ' '); if (ptr == NULL) goto wipe; 
			rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "soil_moisture", "%05.2f", atof(ptr));
			rc += xmlTextWriterEndElement(writer);
			rc += xmlTextWriterStartElement(writer, BAD_CAST "light");
			ptr = strchr(ptr, ' '); if (ptr == NULL) goto wipe; 
			rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "R", "%d", atoi(ptr));
			ptr = strchr(ptr, ' '); if (ptr == NULL) goto wipe; 
			rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "G", "%d", atoi(ptr));
			ptr = strchr(ptr, ' '); if (ptr == NULL) goto wipe; 
			rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "B", "%d", atoi(ptr));
			rc += xmlTextWriterEndElement(writer);
			rc += xmlTextWriterStartElement(writer, BAD_CAST "battery");
			ptr = strchr(ptr, ' '); if (ptr == NULL) goto wipe; 
			rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "voltage", "%05.2f", atof(ptr));
			rc += xmlTextWriterEndElement(writer);
			rc += xmlTextWriterEndElement(writer);
			rc += xmlTextWriterEndDocument(writer);

			/* Now specify the POST data */
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, finalbuf);
			printf(finalbuf);

			/* Try to resend any queued data from previous failed attempts */
			if (strlen(queued) > 9)
			{
				res = curl_easy_perform(curl);
				if (0 == res)
				{
					queued[0] = 0;
				}
			}

			/* Perform the request, res will get the return code */
			res = curl_easy_perform(curl); // TODO deal with error here!!!

			int endptr = strlen(queued);

			if (0 != res) 
			{
				for (i = 0; ((i < sizeof(finalbuf)) && ((endptr + i) < MAXQUEUE)); i++)
				{
					queued[endptr + i] = finalbuf[i];
					// add contents of finalbuf to large backup buffer and go around again
				}
			}
wipe:
			total = 0;
			for (i = 0; i < sizeof(finalbuf); i++)
			{
				webbuf[i] = 0;
				finalbuf[i] = 0;
			}
			pos = 0;
		}
	}
	xmlFreeTextWriter(writer);
}
