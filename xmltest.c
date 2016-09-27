#include <stdio.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>

#define MY_ENCODING "ISO-8859-1"
 
    int rc;
    xmlTextWriterPtr writer;
    xmlBufferPtr buf;

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
    rc = xmlTextWriterStartDocument(writer, NULL, MY_ENCODING, NULL);

    /* Start an element named "EXAMPLE". Since thist is the first
     * element, this will be the root element of the document. */
    rc += xmlTextWriterStartElement(writer, BAD_CAST "data_point");
    rc += xmlTextWriterStartElement(writer, BAD_CAST "one_metre");
    rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "temperature", "%05.2f", 23.1);
    rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "humidity", "%05.2f", 73.47);
    rc += xmlTextWriterEndElement(writer);
    rc += xmlTextWriterStartElement(writer, BAD_CAST "twenty_centimetres");
    rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "temperature", "%05.2f", 23.1);
    rc += xmlTextWriterWriteFormatElement(writer, BAD_CAST "humidity", "%05.2f", 73.47);
    rc += xmlTextWriterEndElement(writer);
    rc += xmlTextWriterEndElement(writer);
    rc += xmlTextWriterEndDocument(writer);

    xmlFreeTextWriter(writer);

