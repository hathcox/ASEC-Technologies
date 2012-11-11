/*
 * Doorway_Server.h
 *
 *  Created on: Nov 5, 2012
 *      Author: haddaway
 */

#ifndef DOORWAY_SERVER_H_
#define DOORWAY_SERVER_H_

void startConnection();
void stopConnection();
int validateUser(char* username, char* password);
void parseMessage();
int validateAuthToken(char* authToken);

#endif /* DOORWAY_SERVER_H_ */
