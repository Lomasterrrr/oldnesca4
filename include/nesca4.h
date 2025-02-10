/*
 *          NESCA4
 *   Сделано от души 2023.
 * Copyright (c) [2023] [lomaster]
 * SPDX-License-Identifier: BSD-3-Clause
 *
*/

#include <cstdint>
#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <cstdlib>
#include <chrono>
#include <future>
#include <termios.h>
#include <unistd.h>
#include <thread>
#include <iomanip>
#include <mutex>
#include <algorithm>
#include <random>
#include <string>
#include <future>
#include <sstream>
#include <set>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <vector>

#include "nescabrute.h"
#include "nescaproc.h"
#include "nescadata.h"
#include "nescalog.h"
#include "nescathread.h"

#include "../include/nescautils.h"

#include "../config/nescaopts.h"
#include "../ncbase/include/getopt.h"
#include "../ncbase/include/json.h"
#include "../ncbase/include/base64.h"
#include "../ncbase/include/binary.h"
#include "../config/compile.h"

#include "../ncsock/include/tcp.h"
#include "../ncsock/include/http.h"
#include "../ncsock/include/utils.h"
#include "../ncsock/include/readpkt.h"
#include "../ncsock/include/dns.h"
#include "../ncsock/include/ftp.h"
#include "../ncsock/include/smtp.h"
#include "../ncsock/include/icmp.h"

/*Угадайте?*/
#define _VERSION "20240115"

void usage(const char* exec);
void version_menu(void);
void nescaend(int success, double res);
void init_bruteforce(void);
void processing_tcp_scan_ports(std::string ip, int port, int result);
std::string format_percentage(double procents);
void fix_time(double time);

void PRENESCASCAN(void);
void NESCASCAN(std::vector<std::string>& temp_vector);
void nesca_scan(const std::string& ip, std::vector<int>ports, const int timeout_ms);
void nesca_ping(const std::string& ip);
void nesca_ddos(u8 proto, u8 type, const u32 daddr, const u32 saddr, const int port, bool ip_ddos);
void nesca_http(const std::string& ip, const u16 port, const int timeout_ms);

struct nescalog_opts
{
  int total, set;
  std::string name;
};

struct nescalog_opts initlog(int total, int set, const std::string &name);
void nesca_log(struct nescalog_opts *nlo, int complete);

template<typename Func, typename... Args> void
nesca_group_execute(struct nescalog_opts *nlo, int threads, std::vector<std::string> group, Func&& func, Args&&... args);

void importfile(void);
void parse_args(int argc, char** argv);
void pre_check(void);
void process_port(const std::string& ip, std::vector<uint16_t> ports, int port_type);
void get_dns_thread(std::string ip);
int count_map_vector(const std::unordered_map<std::string, std::vector<int>>& map, const std::string& key);
std::vector<std::string> resolvhosts(bool datablocks, std::vector<std::string> hosts);

const struct
option long_options[] = {
  {"udp", no_argument, 0, 55},
  {"badsum", no_argument, 0, 65},
  {"PU", required_argument, 0, 18},
  {"PY", required_argument, 0, 13},
  {"sctp-init", no_argument, 0, 14},
  {"sctp-cookie", no_argument, 0, 17},
  {"ack", required_argument, 0, 42},
  {"gdel", required_argument, 0, 58},
  {"ipopt", required_argument, 0, 59},
  {"data", required_argument, 0, 38},
  {"window", required_argument, 0, 24},
  {"http-timeout", required_argument, 0, 22},
  {"psh", no_argument, 0, 3},
  {"script", required_argument, 0, 4},
  {"no-verbose", no_argument, 0, 6},

  {"PR", no_argument, 0, 8},
  {"adler32", no_argument, 0, 15},
  
  /*
  {"icmp", required_argument, 0, 16},
  {"reqnum", required_argument, 0, 10},
  {"TDD", required_argument, 0, 9},
  */

  {"no-scan", no_argument, 0, 46},
  {"print-color", required_argument, 0, 78},
  {"data-len", required_argument, 0, 79},
  {"scanflags", required_argument, 0, 21},
  {"frag", required_argument, 0, 20},
  {"threads", required_argument, 0, 'T'},
  {"delay", required_argument, 0, 'd'},
  {"import", required_argument, 0, 23},
  {"find", required_argument, 0, 19},
  {"random-ip", required_argument, 0, 5},
  {"data-string", required_argument, 0, 56},
  {"ping-log", required_argument, 0, 90},
  {"my-life-my-rulez", no_argument, 0, 53},


  {"brute-delay", required_argument, 0, 47},
  {"brute-login", required_argument, 0, 12},
  {"brute-pass", required_argument, 0, 11},
  {"brute-timeout", required_argument, 0, 31},
  {"brute-threads", required_argument, 0, 39},
  {"brute-maxcon", required_argument, 0, 41},
  {"brute-attempts", required_argument, 0, 64},
  {"no-brute", required_argument, 0, 44},


  {"thread-on-port", no_argument, 0, 48},
  {"negatives", required_argument, 0, 76},
  {"TP", required_argument, 0, 57},
  {"debug", no_argument, 0, 27},
  {"er", no_argument, 0, 28},
  {"TH", required_argument, 0, 50},
  {"import-color", required_argument, 0, 54},
  {"null", no_argument, 0, 91},
  {"fin", no_argument, 0, 92},
  {"xmas", no_argument, 0, 93},
  {"http-response", no_argument, 0, 51},
  {"scan-timeout", required_argument, 0, 101},
  {"no-ping", no_argument, 0, 29},
  {"no-proc", no_argument, 0, 95},
  {"no-color", no_argument, 0, 26},
  {"packet-trace", no_argument, 0, 96},
  {"no-resolv", no_argument, 0, 'n'},
  {"help", no_argument, 0, 'h'},
  {"version", no_argument, 0, 'v'},
  {"ports", no_argument, 0, 'p'},
  {"db", no_argument, 0, 7},
  {"error", no_argument, 0, 25},
  {"html", required_argument, 0, 'l'},
  {"TD", required_argument, 0, 52},
  {"PS", required_argument, 0, 80},
  {"PA", required_argument, 0, 81},
  {"PE", no_argument, 0, 82},
  {"PI", no_argument, 0, 86},
  {"json", required_argument, 0, 43},
  {"PM", no_argument, 0, 87},
  {"sport", required_argument, 0, 36},
  {"ttl", required_argument, 0, 37},
  {"robots", no_argument, 0, 67},
  {"hikshots", required_argument, 0, 35},
  {"sitemap", no_argument, 0, 68},

  {"maxg-ping", required_argument, 0, 60},
  {"maxg-scan", required_argument, 0, 61},

  {"exclude", required_argument, 0, 62},
  {"excludefile", required_argument, 0, 63},
  {"max-ping", no_argument, 0, 88},
  {"random-dns", required_argument, 0, 32},
  {"saddr", required_argument, 0, 34},
  {"ack", no_argument, 0, 89},
  {"window", no_argument, 0, 94},
  {"maimon", no_argument, 0, 97},
  {"resol-delay", required_argument, 0, 40},
  {"speed", required_argument, 0, 'S'},
  {"resol-port", required_argument, 0, 33},
  {"ping-timeout", required_argument, 0, 49},
  {0,0,0,0}
};
