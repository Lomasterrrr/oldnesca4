/*
 *          NESCA4
 *   Сделано от души 2023.
 * Copyright (c) [2023] [lomaster]
 * SPDX-License-Identifier: BSD-3-Clause
 *
*/

#include "../include/nescaengine.h"
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <netinet/in.h>
#include "../ncsock/include/eth.h"
#include "../ncsock/include/sctp.h"
#include "../ncsock/include/utils.h"
#include "../ncsock/include/arp.h"
#include "../ncsock/include/utils.h"
#include <chrono>
#include <sys/socket.h>
#include <unistd.h>

u8 *tcp_probe(NESCADATA *n, const ipnesca_t dst, u16 dport, u8 type, u32 *packetlen)
{
  u8 tcpflags = 0;
  u8 *res = NULL;
  bool df = true;
  u16 destport;

  switch (type) {
    case TCP_PING_ACK:
      tcpflags = TCP_FLAG_ACK;
      destport = n->ack_ping_ports[n->temp_ack_port];
      n->temp_ack_port++;
      break;
    case TCP_PING_SYN:
      tcpflags = TCP_FLAG_SYN;
      destport = n->syn_ping_ports[n->temp_syn_port];
      n->temp_syn_port++;
      break;
    default:
      tcpflags = n->tcpflags;
      destport = dport;
      break;
  }

  if (!n->customsport)
    n->sport = random_srcport();
  if (!n->customttl)
    n->ttl = -1;

  if (n->frag_mtu > 0)
    df = false;

  res = tcp4_build_pkt(n->src, dst, n->ttl, random_u16(), 0, df,
      n->ip_options, n->ipoptslen, n->sport, destport, random_u32(),
      0, 0, tcpflags, n->windowlen, 0, NULL, 0, n->pd.data,
      n->pd.datalen, packetlen, n->badsum);

  return res;
}

u8 *icmp_probe(NESCADATA *n, const ipnesca_t dst, u8 type, u32 *packetlen)
{
  u8 *res = NULL;
  bool df = true;
  u8 icmptype = 0;

  if (!n->customttl)
    n->ttl = -1;

  switch (type) {
    case ICMP_PING_ECHO:
      icmptype = ICMP4_ECHO;
      break;
    case ICMP_PING_INFO:
      icmptype = ICMP4_INFO_REQUEST;
      break;
    case ICMP_PING_TIME:
      icmptype = ICMP4_TIMESTAMP;
      break;
  }

  if (n->frag_mtu > 0)
    df = false;

  res = icmp4_build_pkt(n->src, dst, n->ttl, random_u16(), 0, df,
      n->ip_options, n->ipoptslen, random_u16(), random_u16(),
      icmptype, 0, n->pd.data, n->pd.datalen, packetlen, n->badsum);

  return res;
}

u8 *sctp_probe(NESCADATA *n, const ipnesca_t dst, u16 dport, u8 type, u32 *packetlen)
{
  u8 *res = NULL;
  bool df = true;
  char *chunk = NULL;
  int chunklen = 0;
  u32 vtag = 0;

  if (!n->customsport)
    n->sport = random_srcport();
  if (!n->customttl)
    n->ttl = -1;

  switch (type) {
    case SCTP_INIT_PING:
      dport = n->sctp_ping_ports[n->temp_sctp_port];
      n->temp_sctp_port++;
    case SCTP_INIT_SCAN:
      chunklen = sizeof(struct sctp_chunk_hdr_init);
      chunk = (char*)malloc(chunklen);
      sctp_pack_chunkhdr_init(chunk, SCTP_INIT, 0, chunklen, random_u32(), 32768, 10, 2048, random_u32());
      break;
    case SCTP_COOKIE_SCAN:
      chunklen = sizeof(struct sctp_chunk_header_cookie_echo) + 4;
      chunk = (char*)malloc(chunklen);
      *((u32*)((char*)chunk + sizeof(struct sctp_chunk_header_cookie_echo))) = random_u32();
      sctp_pack_chunkhdr_cookie_echo(chunk, SCTP_COOKIE_ECHO, 0, chunklen);
      vtag = random_u32();
      break;
  }

  if (n->frag_mtu > 0)
    df = false;

  res = sctp4_build_pkt(n->src, dst, n->ttl,
    random_u16(), 0, df, n->ip_options, n->ipoptslen, n->sport, dport, vtag, chunk,
    chunklen, n->pd.data, n->pd.datalen, packetlen, n->adler32sum, n->badsum);

  if (chunk)
    free(chunk);
  return res;
}

u8 *udp_probe(NESCADATA *n, const ipnesca_t dst, u16 dport, u8 type, u32 *packetlen)
{
  u8 *res = NULL;
  bool df = true;

  if (type == UDP_PING) {
    dport = n->udp_ping_ports[n->temp_udp_port];
    n->temp_udp_port++;
  }

  if (!n->customsport)
    n->sport = random_srcport();
  if (!n->customttl)
    n->ttl = -1;

  if (n->frag_mtu > 0)
    df = false;

  res = udp4_build_pkt(n->src, dst, n->ttl, random_u16(), 0, df, n->ip_options, n->ipoptslen,
      n->sport, dport, n->pd.data, n->pd.datalen, packetlen, n->badsum);

  return res;
}

u8 *arp_probe(NESCADATA *n, const ipnesca_t dst, u8 type, u32 *packetlen)
{
  u8 *res = NULL;
  ip4_addreth_t daddr, saddr;
  eth_addr_t ethsaddr;

  /* ip source */
  memcpy(saddr.data, &n->src, sizeof(saddr.data));
  /* ip dest */
  memcpy(daddr.data, &dst, sizeof(daddr.data));
  /* mac source */
  memcpy(ethsaddr.data, n->srcmac, ETH_ADDR_LEN);

  res = arp4_build_pkt(ethsaddr, MAC_STRING_TO_ADDR(ETH_ADDR_BROADCAST), ARP_HRD_ETH,
      ARP_PRO_IP, ETH_ADDR_LEN, IP4_ADDR_LEN, ARP_OP_REQUEST, ethsaddr,
      saddr, MAC_STRING_TO_ADDR("\x00\x00\x00\x00\x00\x00"), daddr, packetlen);

  return res;
}

int nescasocket(void)
{
  return (socket(AF_INET, SOCK_RAW, IPPROTO_RAW));
}

bool readping(NESCADATA *n, const ipnesca_t dst, u8 *packet, u8 type)
{
  u8 icmptype;
  u8 tcpflags = 0;

  if (type == ICMP_PING_ECHO || type == ICMP_PING_TIME || type == ICMP_PING_INFO) {
    const struct icmp4_hdr *icmp = (struct icmp4_hdr *)(packet + sizeof(struct eth_hdr) + sizeof(struct ip4_hdr));
    icmptype = icmp->type;
    if (type == ICMP_PING_TIME && icmptype == ICMP4_TIMESTAMPREPLY)
      return true;
    if (type == ICMP_PING_ECHO && icmptype == ICMP4_ECHOREPLY)
      return true;
    if (type == ICMP_PING_INFO && icmptype == ICMP4_INFO_REPLY)
      return true;
  }
  if (type == TCP_PING_ACK || type == TCP_PING_SYN) {
    const struct tcp_hdr *tcp = (struct tcp_hdr*)(packet + sizeof(struct eth_hdr) + sizeof(struct ip4_hdr));
    tcpflags = tcp->th_flags;
    if (type == TCP_PING_ACK && tcpflags == TCP_FLAG_RST)
      return true;
    if (type == TCP_PING_SYN && tcpflags)
      return true;
  }
  if (type == SCTP_INIT_PING) {
    const struct sctp_hdr *sctp = (struct sctp_hdr*)(packet + sizeof(struct eth_hdr) + sizeof(struct ip4_hdr));
    if (sctp->srcport)
      return true;
  }
  if (type == UDP_PING) {
    const struct ip4_hdr *ip = (struct ip4_hdr*)(packet + sizeof(struct eth_hdr));
    if (ip->proto == IPPROTO_ICMP) {
      const struct icmp4_hdr *icmp = (struct icmp4_hdr*)(packet + sizeof(struct eth_hdr) + sizeof(struct ip4_hdr));
      if (icmp->type == 3 && icmp->code == 3)
        return true;
    }
    if (ip->proto == IPPROTO_UDP)
      return true;
  }

  return false;
}

int readscan(const ipnesca_t dst, u8 *packet, u8 type)
{
  if (type == SCTP_INIT_SCAN || type == SCTP_COOKIE_SCAN || type == SCTP_INIT_PING) {
    const struct sctp_hdr *sctp = (struct sctp_hdr*)(packet + sizeof(struct eth_hdr) + sizeof(struct ip4_hdr));
    const struct sctp_chunk_hdr *chunk = (struct sctp_chunk_hdr*)((u8*)sctp + 12);

    if (type == SCTP_INIT_SCAN) {
      if (chunk->type == SCTP_INIT_ACK)
        return PORT_OPEN;
      else if (chunk->type == SCTP_ABORT)
        return PORT_CLOSED;
    }
    if (type == SCTP_COOKIE_SCAN)
      if (chunk->type == SCTP_ABORT)
        return PORT_CLOSED;
  }
  else if (type == UDP_SCAN) {
    const struct ip4_hdr *ip = (struct ip4_hdr*)(packet + sizeof(struct eth_hdr));
    if (ip->proto == IPPROTO_ICMP) {
      const struct icmp4_hdr *icmp = (struct icmp4_hdr*)(packet + sizeof(struct eth_hdr) + sizeof(struct ip4_hdr));
      if (icmp->type == 3 && icmp->code == 3)
        return PORT_CLOSED;
    }
    else if (ip->proto == IPPROTO_UDP) /* its real? */
      return PORT_OPEN;
    else
      return PORT_FILTER;
  }
  else {
    const struct tcp_hdr *tcp = (struct tcp_hdr*)(packet + sizeof(struct eth_hdr) + sizeof(struct ip4_hdr));
    switch (type) {
      case TCP_MAIMON_SCAN: {
        if (tcp->th_flags == TCP_FLAG_RST)
          return PORT_CLOSED;
        return PORT_OPEN_OR_FILTER;
      }
      case TCP_PSH_SCAN:
      case TCP_FIN_SCAN:
      case TCP_XMAS_SCAN:
      case TCP_NULL_SCAN: {
        if (tcp->th_flags == TCP_FLAG_RST)
          return PORT_CLOSED;
        return PORT_OPEN;
      }
      case TCP_WINDOW_SCAN: {
        if (tcp->th_flags == TCP_FLAG_RST) {
          if (tcp->th_win > 0)
            return PORT_OPEN;
          else
            return PORT_CLOSED;
          return PORT_FILTER;
        }
      }
      case TCP_ACK_SCAN: {
        if (tcp->th_flags == TCP_FLAG_RST)
          return PORT_NO_FILTER;
        return PORT_FILTER;
      }
      default: {
        switch (tcp->th_flags) {
          case 0x12:
            return PORT_OPEN;
          case 0x1A:
            return PORT_OPEN;
          case TCP_FLAG_RST:
            return PORT_CLOSED;
          default:
            return PORT_FILTER;
        }
      }
    }
  }

  return PORT_ERROR;
}

u8* recvpacket(NESCADATA *n, ipnesca_t dst, u8 type, int timeout_ms, double *rtt)
{
  struct readfiler rf;
  struct sockaddr_in dest;
  int read = -1;
  u8 *res;
  u8 proto = 0;
  u8 secondproto = 0;

  switch (type) {
    case ICMP_PING_ECHO:
    case ICMP_PING_INFO:
    case ICMP_PING_TIME:
      proto = IPPROTO_ICMP;
      break;
    case SCTP_INIT_SCAN:
    case SCTP_INIT_PING:
    case SCTP_COOKIE_SCAN:
      proto = IPPROTO_SCTP;
      break;
    case UDP_PING:
    case UDP_SCAN:
      proto = IPPROTO_UDP;
      secondproto = IPPROTO_ICMP;
      break;
    default:
      proto = IPPROTO_TCP;
      break;
  }

  dest.sin_family = AF_INET;
  dest.sin_addr.s_addr = dst;
  rf.protocol = proto;
  rf.second_protocol = secondproto;
  rf.ip = (struct sockaddr_storage*)&dest;

  res = (u8*)calloc(RECV_BUFFER_SIZE, sizeof(u8));
  if (!res)
    return NULL;

  auto start_time = std::chrono::high_resolution_clock::now();
  read = read_packet(&rf, timeout_ms, &res);
  auto end_time = std::chrono::high_resolution_clock::now();
  if (read == -1) {
    free(res);
    return NULL;
  }
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

  *rtt = duration.count() / 1000.0;
  return res;
}

double nescaping(NESCADATA *n, ipnesca_t dst, u8 type)
{
  ssize_t send;
  double res;
  u8 *packet;
  int fd = 0;

  fd = nescasocket();
  if (fd == -1)
    return -1;
  send = sendprobe(fd, n, dst, 0, type);
  close(fd);
  if (send == -1)
    return -1;
  packet = recvpacket(n, dst, type, n->ping_timeout, &res);
  if (!packet)
    return -1;
  if (!readping(n, dst, packet, type))
    return -1;

  return res;
}

ssize_t sendprobe(int fd, NESCADATA *n, const ipnesca_t dst,
    u16 dport, u8 type)
{
  struct sockaddr_in dest;
  u32 packetlen;
  u8 *packet;

  dest.sin_addr.s_addr = dst;
  dest.sin_family = AF_INET;

  switch (type) {
    case ICMP_PING_ECHO:
    case ICMP_PING_INFO:
    case ICMP_PING_TIME:
      packet = icmp_probe(n, dst, type, &packetlen);
      break;
    case SCTP_COOKIE_SCAN:
    case SCTP_INIT_SCAN:
    case SCTP_INIT_PING:
      packet = sctp_probe(n, dst, dport, type, &packetlen);
      break;
    case UDP_PING:
    case UDP_SCAN:
      packet = udp_probe(n, dst, dport, type, &packetlen);
      break;
    case ARP_PING:
      packet = arp_probe(n, dst, type, &packetlen);
      break;
    default:
      packet = tcp_probe(n, dst, dport, type, &packetlen);
      break;
  }

  return (ip4_send(NULL, fd, &dest, n->frag_mtu, packet, packetlen));
}
