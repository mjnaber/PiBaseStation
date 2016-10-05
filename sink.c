#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
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
	//tty_fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);
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

int main()
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
	char queued [MAXQUEUE]; // Allocate 1M of RAM for buffering failed transmission attempts until network allows CURL to succeed

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

	FILE *curllog = fopen("/SD/curllog.txt", "a"); // Both log files will need to be on the SD card, NOT the RAM disk
	FILE *xmllog = fopen("/SD/xmllog.txt", "a");   // Should probably be passed in on the command line

	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, curllog);
	curl_easy_setopt(curl, CURLOPT_URL, "http://users.aber.ac.uk/mjn/zambiatestdata.php");
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20); // Timeout CURL after 20 seconds (plenty of time for the network to respond)

	while (1)
	{
		/* Create a new XML buffer*/
		buf = xmlBufferCreate();
		if (buf == NULL) {
			printf("Error creating the xml buffer\n");
			return -1;
		}

		/* Create a new XmlWriter */ 
		writer = xmlNewTextWriterMemory(buf, 0);
		if (writer == NULL) {
			printf("Error creating the xml writer\n");
			return -1;
		}

		/* Start the document with the xml default for the version,
		 * encoding ISO 8859-1 */
		rc = xmlTextWriterStartDocument(writer, NULL, MY_ENCODING, NULL);
		if (rc < 0) {
			printf
				("Error at xmlTextWriterStartDocument\n");
			return -1;
		}

		xmlTextWriterSetIndent(writer, 1);
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

			/* Start an element named "EXAMPLE". Since thist is the first
			 * element, this will be the root element of the document. */
			rc += xmlTextWriterStartElement(writer, BAD_CAST "data_point");
			rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "node_ID", "%d", atoi(ptr));
			rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "timestamp", "%s", (const char*) ctime(&result));
			//ptr = strchr(ptr, ' '); if (ptr == NULL) goto wipe; 
			//rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "message_type", "%d", atoi(ptr++));
			rc += xmlTextWriterStartElement(writer, BAD_CAST "one_metre");
			ptr = strchr(ptr, ' '); if (ptr == NULL) goto wipe; 						// advance to next space (used as a separator)
			rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "temperature", "%05.2f", atof(ptr++)); 	// post-increment ptr to make sure we move to
			ptr = strchr(ptr, ' '); if (ptr == NULL) goto wipe; 						// next separator and give up if we don't
			rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "humidity", "%05.2f", atof(ptr++));	// find another space (corrupt string)
			rc += xmlTextWriterEndElement(writer);								// keep going through all the entries...
			rc += xmlTextWriterStartElement(writer, BAD_CAST "twenty_centimetres");
			ptr = strchr(ptr, ' '); if (ptr == NULL) goto wipe; 
			rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "temperature", "%05.2f", atof(ptr++));
			ptr = strchr(ptr, ' '); if (ptr == NULL) goto wipe; 
			rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "humidity", "%05.2f", atof(ptr++));
			rc += xmlTextWriterEndElement(writer);
			rc += xmlTextWriterStartElement(writer, BAD_CAST "ground_level");
			ptr = strchr(ptr, ' '); if (ptr == NULL) goto wipe; 
			rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "temperature", "%05.2f", atof(ptr++));
			ptr = strchr(ptr, ' '); if (ptr == NULL) goto wipe; 
			rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "humidity", "%05.2f", atof(ptr++));
			rc += xmlTextWriterEndElement(writer);
			rc += xmlTextWriterStartElement(writer, BAD_CAST "soil");
			ptr = strchr(ptr, ' '); if (ptr == NULL) goto wipe; 
			rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "temperature", "%05.2f", atof(ptr++));
			ptr = strchr(ptr, ' '); if (ptr == NULL) goto wipe; 
			rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "soil_moisture", "%05.2f", atof(ptr++));
			rc += xmlTextWriterEndElement(writer);
			rc += xmlTextWriterStartElement(writer, BAD_CAST "light");
			ptr = strchr(ptr, ' '); if (ptr == NULL) goto wipe; 
			rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "R", "%d", atoi(ptr++));
			ptr = strchr(ptr, ' '); if (ptr == NULL) goto wipe; 
			rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "G", "%d", atoi(ptr++));
			ptr = strchr(ptr, ' '); if (ptr == NULL) goto wipe; 
			rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "B", "%d", atoi(ptr++));
			rc += xmlTextWriterEndElement(writer);
			rc += xmlTextWriterStartElement(writer, BAD_CAST "battery");
			ptr = strchr(ptr, ' '); if (ptr == NULL) goto wipe; 
			rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "voltage", "%05.2f", atof(ptr++));
			rc += xmlTextWriterEndElement(writer);
			rc += xmlTextWriterEndElement(writer);
			rc += xmlTextWriterEndDocument(writer);

			/* Try to resend any queued data from previous failed attempts before sending new data */
			if (strlen(queued) > 9)
			{
				/* Specify the POST data */
				sprintf(finalbuf, "data=%s", queued);
				curl_easy_setopt(curl, CURLOPT_POSTFIELDS, finalbuf);

				res = curl_easy_perform(curl);
				if (0 == res)
				{
					printf("Successfully cleared queue\n");
					queued[0] = 0;
				}
			}

			fprintf(xmllog, "%s", buf->content); 			// Write XML to the log file

			/* Specify the POST data */
			sprintf(finalbuf, "data=%s", buf->content);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, finalbuf);

			/* Perform the request, res will get the return code */
			res = curl_easy_perform(curl);

			int endptr = strlen(queued);

			if (0 != res) // If curl fails then queue data 
			{
				printf("Queuing Data...\n");
				for (i = 0; ((i < sizeof(finalbuf)) && ((endptr + i) < MAXQUEUE)); i++)
				{
					queued[endptr + i] = finalbuf[i];
					// add contents of finalbuf to large backup buffer and go around again
				}
			}
			else
			{
				printf("Sent packet via CURL\n");
			}
wipe:
			xmlFreeTextWriter(writer);
			xmlBufferFree(buf);

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
