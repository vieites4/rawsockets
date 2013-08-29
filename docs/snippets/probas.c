unsigned char src_mac[6] = {0x1C, 0x75, 0x08, 0x09, 0xF7, 0xAE};
unsigned char dest_mac[6] = {0x1C, 0x75, 0x08, 0x09, 0xF7, 0xAE};//0x00, 0x04, 0x75, 0xC8, 0x28, 0xE5};
/*RAW communication*/
socket_address.sll_family   = PF_PACKET;	// pon que debería ser sempre AF_PACKET
socket_address.sll_protocol = htons(ETH_P_ALL);	// htons(0x0707);
socket_address.sll_ifindex  = 2; //numero da interface
socket_address.sll_hatype   = ARPHRD_IEEE80211;//ARPHRD_ETHER; //tipo de cabeceira. solo afecta á recepcion //#define ARPHRD_IEEE80211 801
socket_address.sll_pkttype  = PACKET_OTHERHOST; // tipo de paquete. solo afecta á recepcion
socket_address.sll_halen    = ETH_ALEN;		//lonxitude da cabeceira
/*MAC - begin*/ //mac destino 0x1C, 0x75, 0x08, 0x09, 0xF7, 0xAE};
/*socket_address.sll_addr[0]  = 0x00;
socket_address.sll_addr[1]  = 0x04;
socket_address.sll_addr[2]  = 0x75;
socket_address.sll_addr[3]  = 0xC8;
socket_address.sll_addr[4]  = 0x28;
socket_address.sll_addr[5]  = 0xE5;*/
socket_address.sll_addr[0]  = 0x1c;
socket_address.sll_addr[1]  = 0x75;
socket_address.sll_addr[2]  = 0x08;
socket_address.sll_addr[3]  = 0x09;
socket_address.sll_addr[4]  = 0xf7;
socket_address.sll_addr[5]  = 0xae;
socket_address.sll_addr[6]  = 0x00;/*not used*/
socket_address.sll_addr[7]  = 0x00;/*not used*/



memcpy((void*)buf, (void*)dest_mac, ETH_ALEN);
memcpy((void*)(buf+ETH_ALEN), (void*)src_mac, ETH_ALEN);
int j;
for (j = 0; j < 1500; j++) {
	data[j] = 0;//(unsigned char)((int) (255.0*rand()/(RAND_MAX+1.0)));
}

/*send the packet*/
send_result = sendto(s, buf, ETH_FRAME_LEN, 0,
	      (struct sockaddr*)&socket_address, sizeof(socket_address));

if (send_result == -1) {  perror ("The following error occurred"); }
//return(&socket_address);

}
