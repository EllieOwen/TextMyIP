
/*
 * ip_monitor.c
 * 
 * Copyright 2016 root <root@raspberrypi>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */
 /***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2015, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at https://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/ 
/* <DESC>
 * Shows how the write callback function can be used to download data into a
 * chunk of memory instead of storing it in a file.
 * </DESC>
 */ 
 /* <DESC>
 * SMTP example showing how to send e-mails
 * </DESC>
 */ 
/* This is a simple example showing how to send mail using libcurl's SMTP
 * capabilities. For an example of using the multi interface please see
 * smtp-multi.c.
 *
 * Note that this example requires libcurl 7.20.0 or above.
 */ 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <unistd.h>
#include <config.h>/*contains your private data */

char myIP;
char *myIP_ptr = &myIP ;
int ip_changed = 0;

struct string {
  char *ptr;
  size_t len;
};

void init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size*nmemb;
}

int get_ip(void)
{
	CURL *curl;
	CURLcode res;
	curl_global_init(CURL_GLOBAL_DEFAULT);
	
    curl = curl_easy_init();
    if(curl) {
      struct string s;
      init_string(&s);
		
	  curl_easy_setopt(curl, CURLOPT_URL, "https://api.ipify.org/?format=json");
	  /*
	   * https://curl.haxx.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
	   * CURLcode curl_easy_setopt(CURL *handle, CURLOPT_WRITEFUNCTION, write_callback);
	   * size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata);*/	   
	  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
	  /*
	   * https://curl.haxx.se/libcurl/c/CURLOPT_WRITEDATA.html
	   * CURLcode curl_easy_setopt(CURL *handle, CURLOPT_WRITEDATA, void *pointer);
	   */
	  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
	  
	  res = curl_easy_perform(curl);
	  
	  if(strncmp(s.ptr, myIP_ptr, 26) != 0){
	    myIP_ptr = s.ptr;
	    ip_changed = 1;
	  }
	  /*printf("myIP %s\n", myIP);*/
	  printf("%s\n",s);
      printf("My pointer %s\n", s.ptr);
      printf("length is %d\n", s.len);
      printf("%X\n", &s);
      /*free(s.ptr);*/
	  curl_global_cleanup();
	  printf("myIP inside func %s\n", myIP_ptr);
	}
	return(0);	
}

 
 
static const char *payload_text[] = {
  "Date: Mon, 29 Nov 2010 21:54:29 +1100\r\n",
  "To: " TO "\r\n",
  "From: " FROM "(Example User)\r\n",
  "Cc: " CC "(Another example User)\r\n",
  "Message-ID: <dcd7cb36-11db-487a-9f3a-e652a9458efd@"
  "rfcpedant.example.org>\r\n",
  "Subject: My Public IP Address\r\n",
  "\r\n", /* empty line to divide headers from body, see RFC5322 */ 
  "The body of the message starts here. Will be replaced by function.\r\n",
  "\r\n",
  "\r\n",
  "\r\n",
  NULL
};

/*char mybody;
char *mybody_ptr = &mybody;*/

struct upload_status {
  int lines_read;
};
 
static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
{
  struct upload_status *upload_ctx = (struct upload_status *)userp;
  const char *data;
  payload_text[7] = myIP_ptr;
  if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
    return 0;
  }

  data = payload_text[upload_ctx->lines_read];
 
  if(data) {
    size_t len = strlen(data);
    memcpy(ptr, data, len);
    upload_ctx->lines_read++;
 
    return len;
  }
 
  return 0;
}

int send_text(void) 
{
  CURL *curl;
  CURLcode res = CURLE_OK;
  struct curl_slist *recipients = NULL;
  struct upload_status upload_ctx;
 
  upload_ctx.lines_read = 0;
 
  curl = curl_easy_init();
  if(curl) {
    /* This is the URL for your mailserver */ 
    curl_easy_setopt(curl, CURLOPT_USERNAME, USERNAME);/*your email username*/
    curl_easy_setopt(curl, CURLOPT_PASSWORD, PASSWORD);/*your email password*/
    curl_easy_setopt(curl, CURLOPT_URL, URL);/*your email server url*/
 
    /* Note that this option isn't strictly required, omitting it will result
     * in libcurl sending the MAIL FROM command with empty sender data. All
     * autoresponses should have an empty reverse-path, and should be directed
     * to the address in the reverse-path which triggered them. Otherwise,
     * they could cause an endless loop. See RFC 5321 Section 4.5.5 for more
     * details.
     */ 
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, FROM);
 
    /* Add two recipients, in this particular case they correspond to the
     * To: and Cc: addressees in the header, but they could be any kind of
     * recipient. */ 
    recipients = curl_slist_append(recipients, TO);
    recipients = curl_slist_append(recipients, CC);
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
 
    /* We're using a callback function to specify the payload (the headers and
     * body of the message). You could just use the CURLOPT_READDATA option to
     * specify a FILE pointer to read from. */ 
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
    curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
 
    /* Send the message */
    res = curl_easy_perform(curl);
 
    /* Check for errors */ 
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
 
    /* Free the list of recipients */ 
    curl_slist_free_all(recipients);
 
    /* curl won't send the QUIT command until you call cleanup, so you should
     * be able to re-use this connection for additional messages (setting
     * CURLOPT_MAIL_FROM and CURLOPT_MAIL_RCPT as required, and calling
     * curl_easy_perform() again. It may not be a good idea to keep the
     * connection open for a very long time though (more than a few minutes
     * may result in the server timing out the connection), and you do want to
     * clean up in the end.
     */ 
    curl_easy_cleanup(curl);
  }
 
  return (int)res;
}

int main(void)
{	
	
	while(1){
	  get_ip();
	    printf("myIP in main = %s\n", myIP_ptr);
	  if(ip_changed){
	  printf("ip changed %d\n", ip_changed);
	    send_text();
	    ip_changed = 0;
	  }
	  
	  sleep(3600);/*checks once an hour*/
	}
	
	return 0;
}

