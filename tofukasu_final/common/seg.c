#include "./seg.h"

/*----------------private function declaration----------------*/
void add_file_to_list(file_t **head, file_t *new_file);


/*-----------------------------------------------------------------------*/
/*----------------functions to send & receive p2p segment----------------*/
/*-----------------------------------------------------------------------*/
//send p2p packet
//@param:
int send_p2p_segment(int connection, p2p_seg_t *packet){
	char bufstart[2];
	char bufmid[2];
	char bufend[2]; 
	bufstart[0] = '!';
	bufstart[1] = '&';
	bufmid[0]= '!';
	bufmid[1]= '^';
	bufend[0] = '!';
	bufend[1] = '#';
	if (send(connection, bufstart, 2, 0) < 0) {
		return -1;
	}
	if (send(connection, packet, sizeof(p2p_seg_t), 0) < 0) {
		return -1;
	}
	if (send(connection, bufmid, 2, 0) < 0) {
		return -1;
	}
	if (send(connection, packet->data, packet->length, 0) < 0) {
		return -1;
	}
	if (send(connection, bufend, 2, 0) < 0) {
		return -1;
	}
	return 1;
}

//receive p2p packet
int receive_p2p_segment(int connection, p2p_seg_t* packet)
{
	char buf[sizeof(p2p_seg_t) + 2];
	char c;
	int idx = 0;

	char *data;

	/*
	 * state can be 0,1,2,3;
	 * 0 starting point
	 * 1 '!' received
	 * 2 '&' received, start receiving segment
	 * 3 '!' received,
	 * 4 '#' received, finish receiving segment
	 * */
	int state = 0;
	while (recv(connection, &c, 1, 0) > 0) {
		if (state == 0) {
			if (c == '!')
				state = 1;
		} else if (state == 1) {
			if (c == '&')
				state = 2;
			else
				state = 0;
		} else if (state == 2) {
			if (c == '!') {
				buf[idx] = c;
				idx++;
				state = 3;
			} else {
				buf[idx] = c;
				idx++;
			}
		} else if (state == 3) {
			if (c == '^') {
				buf[idx] = c;
				idx++;
				state = 4;
				memcpy(packet, buf, sizeof(p2p_seg_t));

				idx = 0;
				data  = malloc(sizeof(char) * (packet->length+2));
			} else if (c == '!') {
				buf[idx] = c;
				idx++;
			} else {
				buf[idx] = c;
				idx++;
				state = 2;
			}
		} else if (state == 4){
			if(c == '!'){
				data[idx] = c;
				idx++;
				state = 5;
			}else {
				data[idx] = c;
				idx++;
			}
		} else if(state == 5){
			if (c == '#') {
				data[idx] = c;
				idx++;
				state = 0;

				data[packet->length+1] = '\0';
				data[packet->length] = '\0';
				packet->data = data;

				data = NULL;
				idx = 0;

				return 1;
			} else if (c == '!') {
				data[idx] = c;
				idx++;
			} else {
				data[idx] = c;
				idx++;
				state = 4;
			}
		}
	}
	return -1;
}




/*-----------------------------------------------------------------------*/
/*----------------functions to send & receive p2t segment----------------*/
/*-----------------------------------------------------------------------*/
//send p2t_segment
int send_p2t_segment(int connection, p2t_seg_t* packet){
	char bufstart[2];
	char bufheadend[2];
	char buffileend[2];
	char bufend[2]; 
	bufstart[0] = '!';
	bufstart[1] = '&';
	bufheadend[0]= '!';
	bufheadend[1]= '^';
	buffileend[0]= '!';
	buffileend[1]= '$';
	bufend[0] = '!';
	bufend[1] = '#';
	if (send(connection, bufstart, 2, 0) < 0) {
		return -1;
	}
	if (send(connection, packet, sizeof(p2t_seg_t), 0) < 0) {
		return -1;
	}
	if (send(connection, bufheadend, 2, 0) < 0) {
		return -1;
	}
	file_t *cur = packet->filetable.head;
	while(cur!=NULL){
		if (send(connection, cur, sizeof(file_t), 0) < 0) {
			return -1;
		}
		if (send(connection, buffileend, 2, 0) < 0) {
			return -1;
		}
		cur = cur->next;
	}
	if (send(connection, bufend, 2, 0) < 0) {
		return -1;
	}
	return 1;
}

//receive p2t_segment
int receive_p2t_segment(int connection, p2t_seg_t* packet)
{
	char buf[sizeof(p2t_seg_t) + 2];
	char c;
	int idx = 0;

	file_t *file_des;
	char tmp_data[sizeof(file_t) + 2];

	/*
	 * state can be 0,1,2,3;
	 * 0 starting point
	 * 1 '!' received
	 * 2 '&' received, start receiving segment
	 * 3 '!' received,
	 * 4 '#' received, finish receiving segment
	 * */
	int state = 0;
	while (recv(connection, &c, 1, 0) > 0) {
		if (state == 0) {
			if (c == '!')
				state = 1;
		} else if (state == 1) {
			if (c == '&')
				state = 2;
			else
				state = 0;
		} else if (state == 2) {
			if (c == '!') {
				buf[idx] = c;
				idx++;
				state = 3;
			} else {
				buf[idx] = c;
				idx++;
			}
		} else if (state == 3) {
			if (c == '^') {
				buf[idx] = c;
				idx++;
				state = 4;
				memcpy(packet, buf, sizeof(p2t_seg_t));
				packet->filetable.head = NULL;

				idx = 0;
			} else if (c == '!') {
				buf[idx] = c;
				idx++;
			} else {
				buf[idx] = c;
				idx++;
				state = 2;
			}
		} else if (state == 4){
			if (c == '!') {
				tmp_data[idx] = c;
				idx++;
				state = 5;
			} else {
				tmp_data[idx] = c;
				idx++;
			}
		} else if (state == 5){
			if (c == '$') {
				tmp_data[idx] = c;
				idx++;
				state = 4;

				file_des = malloc(sizeof(file_t));
				memcpy(file_des, tmp_data, sizeof(file_t));
				file_des->next =NULL;
			
				add_file_to_list( &(packet->filetable.head), file_des);

				//for storing next file_t
				idx = 0;
				file_des  = NULL;
				memset(tmp_data, 0, sizeof(file_t)+2);
			} else if (c == '#'){
				tmp_data[idx] = c;
				idx++;
				state = 0;

				idx = 0;

				return 1;
			} else if (c == '!') {
				tmp_data[idx] = c;
				idx++;
			} else {
				tmp_data[idx] = c;
				idx++;
				state = 4;
			}
		}
	}
	return -1;
}




/*-----------------------------------------------------------------------*/
/*----------------functions to send & receive t2p segment----------------*/
/*-----------------------------------------------------------------------*/

//send t2p_segment
int send_t2p_segment(int connection, t2p_seg_t* packet){
	char bufstart[2];
	char bufheadend[2];
	char buffileend[2];
	char bufend[2]; 
	bufstart[0] = '!';
	bufstart[1] = '&';
	bufheadend[0]= '!';
	bufheadend[1]= '^';
	buffileend[0]= '!';
	buffileend[1]= '$';
	bufend[0] = '!';
	bufend[1] = '#';
	if (send(connection, bufstart, 2, 0) < 0) {
		return -1;
	}
	if (send(connection, packet, sizeof(t2p_seg_t), 0) < 0) {
		return -1;
	}
	if (send(connection, bufheadend, 2, 0) < 0) {
		return -1;
	}
	file_t *cur = packet->filetable.head;
	while(cur!=NULL){
		if (send(connection, cur, sizeof(file_t), 0) < 0) {
			return -1;
		}
		if (send(connection, buffileend, 2, 0) < 0) {
			return -1;
		}
		cur = cur->next;
	}
	if (send(connection, bufend, 2, 0) < 0) {
		return -1;
	}
	return 1;
}

//receive t2p_segment
int receive_t2p_segment(int connection, t2p_seg_t* packet)
{
	char buf[sizeof(t2p_seg_t) + 2];
	char c;
	int idx = 0;

	file_t *file_des;
	char tmp_data[sizeof(file_t) + 2];

	/*
	 * state can be 0,1,2,3;
	 * 0 starting point
	 * 1 '!' received
	 * 2 '&' received, start receiving segment
	 * 3 '!' received,
	 * 4 '#' received, finish receiving segment
	 * */
	int state = 0;
	while (recv(connection, &c, 1, 0) > 0) {
		if (state == 0) {
			if (c == '!')
				state = 1;
		} else if (state == 1) {
			if (c == '&')
				state = 2;
			else
				state = 0;
		} else if (state == 2) {
			if (c == '!') {
				buf[idx] = c;
				idx++;
				state = 3;
			} else {
				buf[idx] = c;
				idx++;
			}
		} else if (state == 3) {
			if (c == '^') {
				buf[idx] = c;
				idx++;
				state = 4;
				memcpy(packet, buf, sizeof(t2p_seg_t));
				packet->filetable.head = NULL;

				idx = 0;
			} else if (c == '!') {
				buf[idx] = c;
				idx++;
			} else {
				buf[idx] = c;
				idx++;
				state = 2;
			}
		} else if (state == 4){
			if (c == '!') {
				tmp_data[idx] = c;
				idx++;
				state = 5;
			} else {
				tmp_data[idx] = c;
				idx++;
			}
		} else if (state == 5){
			if (c == '$') {
				tmp_data[idx] = c;
				idx++;
				state = 4;

				file_des = malloc(sizeof(file_t));
				memcpy(file_des, tmp_data, sizeof(file_t));
				file_des->next =NULL;
			
				add_file_to_list( &(packet->filetable.head), file_des);

				//for storing next file_t
				idx = 0;
				file_des  = NULL;
				memset(tmp_data, 0, sizeof(file_t)+2);
			} else if (c == '#'){
				tmp_data[idx] = c;
				idx++;
				state = 0;

				idx = 0;

				return 1;
			} else if (c == '!') {
				tmp_data[idx] = c;
				idx++;
			} else {
				tmp_data[idx] = c;
				idx++;
				state = 4;
			}
		}
	}
	return -1;
}


/*----------------functions to manipulate client list----------------*/
void add_file_to_list(file_t **head, file_t *new_file){
	if(*head==NULL){//if head is null, let the new element to be the head
		*head=new_file;
		return;
	}
	file_t *cur = *head;//if head is not null, find the tail element, and let it link to new element
	while(cur->next!=NULL){
		cur=cur->next;
	}
	cur->next=new_file;
}
