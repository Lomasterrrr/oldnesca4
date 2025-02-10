/*
 *          NESCA4
 *   Сделано от души 2023.
 * Copyright (c) [2023] [lomaster]
 * SPDX-License-Identifier: BSD-3-Clause
 *
*/

#ifndef NESCAOPTS_HEADER
#define NESCAOPTS_HEADER

#include <cstdint>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include "compile.h"

#include "../ncsock/include/types.h"

#define SPEED_TYPE_DEFAULT 3
#define DEFAULT_PORTS {80,443}
#define PING_LOG_DEFAULT 20
#define DNS_RESOLV_SOURCE_PORT_DEFUALT 4555
#define NEGATIVES_PATH_DEFAULT "resources/negatives.txt"

#define PATH_FTP_LOGIN_DEFAULT "passwd/ftp_logins.txt"
#define PATH_FTP_PASS_DEFAULT "passwd/ftp_passwords.txt"
#define PATH_SFTP_LOGIN_DEFAULT "passwd/sftp_logins.txt"
#define PATH_SFTP_PASS_DEFAULT "passwd/sftp_passwords.txt"
#define PATH_RTSP_LOGIN_DEFAULT "passwd/rtsp_logins.txt"
#define PATH_RTSP_PASS_DEFAULT "passwd/rtsp_passwords.txt"
#define PATH_HTTP_LOGIN_DEFAULT "passwd/http_logins.txt"
#define PATH_HTTP_PASS_DEFAULT "passwd/http_passwords.txt"
#define PATH_HIKVISION_LOGIN_DEFAULT "passwd/hikvision_logins.txt"
#define PATH_HIKVISION_PASS_DEFAULT "passwd/hikvision_passwords.txt"
#define PATH_SMTP_LOGIN_DEFAULT "passwd/smtp_logins.txt"
#define PATH_SMTP_PASS_DEFAULT "passwd/smtp_passwords.txt"
#define PATH_RVI_LOGIN_DEFAULT "passwd/rvi_logins.txt"
#define PATH_RVI_PASS_DEFAULT "passwd/rvi_passwords.txt"

class arguments_program {
public:
  std::string negatives_path = NEGATIVES_PATH_DEFAULT;
  std::vector<std::string> find_target;

  int type = 6;
  int speed_type = SPEED_TYPE_DEFAULT;
  int count_success_ips;

  std::vector<std::string> exclude;
  bool custom_source_port;
  bool find;
  bool random_dns;
  bool traceroute;
  int random_dns_count;
  bool ns_track;
  int http_threads;

  std::string scriptpath = "";
  bool no_verbose_script = true;

  bool robots_txt;
  bool sitemap_xml;
  std::string screenshots_save_path_cam = "";
  std::unordered_map<std::string, std::string> nesca_negatives;
  std::vector<std::string> new_ping_ip;
  bool no_scan;

  bool custom_tcpflags;
  const char* custom_res_tcpflags;

  bool timeout;
  int timeout_ms;
  bool custom_recv_timeout_ms;
  int recv_timeout_ms;
  int _threads;

  std::vector<int> ports = DEFAULT_PORTS;
  bool no_proc;

  bool debug;
  bool print_errors;
  bool my_life_my_rulez;
  bool get_path_log;
  bool syn_debug;
  bool save_camera_screens;

  bool ping_off;

  bool get_response;

  bool custom_threads_resolv;

  bool random_ip;
  bool no_get_dns;
  bool json_save;
  bool custom_http_threads;
  int http_timeout = 1100;
  int random_ip_count;
  const char* json_save_path;
  int dns_threads;
  int resol_source_port = DNS_RESOLV_SOURCE_PORT_DEFUALT;
  int resol_delay;

  bool ip_scan_import;
  const char* path_ips;

  bool custom_ping;
  int threads_ping;
  bool custom_threads;
  int ping_log = PING_LOG_DEFAULT;

  bool ack_ping;
  bool syn_ping;
  bool echo_ping;
  bool info_ping;
  bool udp_ping;
  bool timestamp_ping;
  bool sctp_ping_init;
  bool max_ping;

  int maxg_ping = 3000;
  int maxg_scan = 2000;

  int group_del = 5;

  /*Don`t touch*/
  bool print_help_menu;
  int count_success_ports;
  int result_success_ports;
  int result_success_ip;
  int threads_temp;
  std::string ports_temp;
  double ping_duration;
  double dns_duration;
  double scan_duration;

  bool thread_on_port;
  bool info_version;
  std::vector<std::string> success_ping_ip;

  std::string path_ftp_login = PATH_FTP_LOGIN_DEFAULT;
  std::string path_ftp_pass = PATH_FTP_PASS_DEFAULT;
  std::string path_sftp_login = PATH_SFTP_LOGIN_DEFAULT;
  std::string path_sftp_pass = PATH_SFTP_PASS_DEFAULT;
  std::string path_rtsp_login = PATH_RTSP_LOGIN_DEFAULT;
  std::string path_rtsp_pass = PATH_RTSP_PASS_DEFAULT;
  std::string path_http_login = PATH_HTTP_LOGIN_DEFAULT;
  std::string path_http_pass = PATH_HTTP_PASS_DEFAULT;
  std::string path_hikvision_login = PATH_HIKVISION_LOGIN_DEFAULT;
  std::string path_hikvision_pass = PATH_HIKVISION_PASS_DEFAULT;
  std::string path_rvi_login = PATH_RVI_LOGIN_DEFAULT;
  std::string path_rvi_pass = PATH_RVI_PASS_DEFAULT;

  int brute_timeout_ms = 25;
  bool off_ftp_brute;
  bool off_sftp_brute;
  bool off_rtsp_brute;
  bool off_rvi_brute;
  bool off_http_brute;
  bool off_hikvision_brute;

  /*Color options*/
  bool color_off;
  bool import_color_scheme;
  const char* path_color_scheme;
};

#endif
