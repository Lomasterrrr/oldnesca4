/*
 *          NESCA4
 *   Сделано от души 2023.
 * Copyright (c) [2023] [lomaster]
 * SPDX-License-Identifier: BSD-3-Clause
 *
*/

#include "../include/nescaproc.h"
#include "../include/nescabrute.h"
#include "../include/nescalog.h"
#include "../ncsock/include/ftp.h"
#include "../ncsock/include/http.h"
#include "../ncsock/include/smtp.h"
#include "../ncbase/include/base64.h"
#include "../ncbase/include/binary.h"
#include "../config/compile.h"

const std::vector<std::string> rtsp_paths = {"Streaming/Channels/101", "h264/ch01/main/av_stream",
                                      "cam/realmonitor?channel=1&subtype=0","live/main",
                                      "av0_0", "mpeg4/ch01/main/av_stream"};

const std::vector<std::string> basic_auth_header = {"401 authorization", "401 unauthorized", "www-authenticate",
                                              "401 unauthorized access denied", "401 unauthorised", "www-authenticate",
                                              "digest realm", "basic realm", "401 Unauthorized"};

const std::vector<std::string> axis_2400_path = {"view/viewer_index.shtml", "view/viewer_index.shtml?", "check_user.cgi",
                                          "view/index2.shtml", "index.shtml","view/indexFrame.shtml","indexFrame.html"};

const std::vector<std::string> network_camera_path ={"ViewerFrame?Mode=", "CgiStart?page=", "admin/index.shtml?"};

const std::vector<std::string> axis_other_path = {"view/viewer_index.shtml", "view/viewer_index.shtml", "check_user.cgi",
                                          "axis-cgi/mjpg/video.cgi", "jpg/image.jpg?size=3", "mjpg/video.mjpg",
                                          "view/viewer_index.shtml", "view/viewer_index.shtml", "check_user.cgi",
                                          "cgi-bin/guest/Video.cgi?", "ISAPI/Security/userCheck", "SnapshotJPEG",
                                          "axis-cgi/com/ptz.cgi?", "mjpg/video.mjpg"};

std::vector<std::string> find_sentences_with_word(const std::string& word, const std::string& input_text)
{
  std::vector<std::string> sentences;
  std::string lowerWord = word;
  std::transform(lowerWord.begin(), lowerWord.end(), lowerWord.begin(), [](u8 c){ return std::tolower(c); });

  std::string lowerText = input_text;
  std::transform(lowerText.begin(), lowerText.end(), lowerText.begin(), [](u8 c){ return std::tolower(c); });

  std::string::size_type pos = lowerText.find(lowerWord);
  while (pos != std::string::npos) {
    std::string::size_type sentenceStart = lowerText.rfind('\n', pos);
    if (sentenceStart == std::string::npos)
      sentenceStart = 0;

    std::string::size_type sentenceEnd = lowerText.find('\n', pos);

    if (sentenceEnd == std::string::npos)
      sentenceEnd = lowerText.length();

    std::string sentence = input_text.substr(sentenceStart, sentenceEnd - sentenceStart);

    if (lowerText.find(lowerWord, sentenceStart) != std::string::npos) {
      sentence.erase(std::remove(sentence.begin(), sentence.end(), '\n'), sentence.end());
      sentence.erase(std::remove(sentence.begin(), sentence.end(), '\r'), sentence.end());
      sentences.push_back(sentence);
    }
    pos = lowerText.find(lowerWord, sentenceEnd);
  }

  return sentences;
}

bool contains_word(const std::string& word, const std::string& sentence)
{
  std::string lowerWord = word;
  std::transform(lowerWord.begin(), lowerWord.end(), lowerWord.begin(), [](u8 c){ return std::tolower(c); });

  std::string lowerSentence = sentence;
  std::transform(lowerSentence.begin(), lowerSentence.end(), lowerSentence.begin(), [](u8 c){ return std::tolower(c); });

  std::string::size_type pos = lowerSentence.find(lowerWord);
  while (pos != std::string::npos) {
    if ((pos > 0 && std::isalpha(lowerSentence[pos - 1])) || 
	(pos + lowerWord.length() < lowerSentence.length() && std::isalpha(lowerSentence[pos + lowerWord.length()])))
      pos = lowerSentence.find(lowerWord, pos + 1);
    else
      return 1;
  }

  return 0;
}

std::string set_target_at_path(const std::string& path)
{
  /* AXIS камеры */
  if (contains_word("operator/basic.shtml", path))
    return CAMERA_AXIS_205;

  for (auto& p : axis_2400_path)
    if (contains_word(p, path))
      return CAMERA_AXIS_2400;

  for (auto& p : axis_other_path)
    if (contains_word(p, path))
      return CAMERA_AXIS;

  /* NETWORK камеры */
  for (auto& p : network_camera_path)
    if (contains_word(p, path))
      return CAMERA_NETWORK;

  if (contains_word("viewer/live/index.html", path)){return CAMERA_VB_M40;}
  if (contains_word("live/index2.html", path)){return CAMERA_BB_SC384;}

  /* Другие камеры */
  if (contains_word("tool.js", path)){return CAMERA_EASY;}
  if (contains_word("command/inquiry.cgi", path)){return CAMERA_SONY;}
  if (contains_word("config/easy_index.cgi", path)){return CAMERA_PANASONIC;}
  if (contains_word("view/getuid.cgi", path)){return CAMERA_PANASONIC_WJ_HD180;}
  if (contains_word("main/cs_motion.asp", path)){return IP_CAMERA_CONTOL;}
  if (contains_word("cgi-bin/data/viostor-220/viostor/viostor.cgi", path)){return NAX;}
  if (contains_word("gui.css", path)){return DIGITAL_VIDEO_SERVER;}
  if (contains_word("config/index.cgi", path)){return CAMERA_PANASONIC_BB_HG;} /* ??? */
  if (contains_word("videostream.cgi", path)){return CAMERA_QCAM;}
  if (contains_word("gui/gui_outer_frame.shtml", path)){return CAMERA_NW;}
  if (contains_word("admin/index.html", path)){return AUTH;}
  if (contains_word("liveview.html", path)){return CAMERA_AVISYS;}
  if (contains_word("js/upfile.js", path)){return CAMERA_FOSCAM;}
  if (contains_word("ssi.cgi/login.htm", path)){return CAMERA_GEO;}
  if (contains_word("src=\\\"webs.cgi", path)){return CAMERA_UA;}
  if (contains_word("app/multi/single.asp", path)){return NETWORK_VIDEO_SYSTEM;}
  if (contains_word("app/live/sim/single.asp", path)){return NETWORK_VIDEO_SYSTEM;}
  if (contains_word("doc/page/login.asp", path)){return WEB_CAMERA_HIKVISION;}
  if (contains_word("login.asp", path)){return HTTP_DIGEST_AUTH;}
  if (contains_word("login", path)){return AUTH;}

  /* Другое */
  if (contains_word("cgi-sys/defaultwebpage.cgi", path)){return CPANEL;}
  if (contains_word("login.html", path)){return AUTH;}
  if (contains_word("cgi-bin/luci", path)){return AUTH_LUCI;}
  if (contains_word("cgi-bin/luci", path)){return AUTH_LUCI;}
  if (contains_word("admin_login", path)){return AUTH_SIMIAN;}
  if (contains_word("ecoremote/index.html", path)){return METEDAS;}
  if (contains_word("password-protected=login", path)){return LIFE_IS_GOOD;}
  if (contains_word("webpages/index.html", path)){return TP_LINK;}
  if (contains_word("k3cloud/html5", path)){return KINGDEE;}
  if (contains_word("https:", path)){return HTTPS_REDIRECT;}

  return "fuck";
}

std::string set_target_at_http_header(const std::string& buffer)
{
  /* Разное */
  for (auto& p : basic_auth_header)
    if (contains_word(p, buffer))
      return HTTP_BASIC_AUTH;

  /* Другие камеры */
  if (contains_word("airos_logo", buffer)){return CAMERA_AIROS;}
  if (contains_word("acti Corporation", buffer)){return CAMERA_ACKTI;}
  if (contains_word("qlikview", buffer)){return CAMERA_QLIK;}
  if (contains_word("webcamxp", buffer)){return CAMERA_WEBCAMXP5;}
  if (contains_word("a valid username/password", buffer)){return CAMERA_WEBCAMXP5;}
  if (contains_word("ieorforefox", buffer)){return CAMERA_IEORFOREFOX;}
  if (contains_word("dvr_remember", buffer)){return NETWORK_VIDEO_CLIENT;}
  if (contains_word("login_chk_usr_pwd", buffer)){return NETWORK_VIDEO_CLIENT;}
  if (contains_word("reecam ip camera", buffer)){return CAMERA_REE;}
  if (contains_word("real-time ip camera monitoring system", buffer)){return IP_CAMERA_MONITORING;}
  if (contains_word("server push mode", buffer)){return IP_CAMERA_MONITORING;}
  if (contains_word("invalid url", buffer)){return INVALID_URL;}
  if (contains_word("vilar ipcamera", buffer)){return CAMERA_VILIAR;}
  if (contains_word("welcome to nginx!", buffer)){return NGNIX;}
  if (contains_word("routeros", buffer)){return ROUTER_OS;}
  if (contains_word("apache2", buffer)){return APACHE2;}
  if (contains_word("cloudflare", buffer)){return CLOUD_FLARE;}
  if (contains_word("www.rvi-cctv.ru", buffer)){return WEB_CAMERA_RVI;}
  if (contains_word("m not a robot", buffer)){return CAPTCHA;}
  if (contains_word("403 error", buffer)){return ERROR_403;}
  if (contains_word("not found", buffer)){return NOT_FOUND_404;}
  if (contains_word("empty", buffer)){return EMPTY_REQUEST;}

  return "fuck";
}

std::string set_target_at_title(const std::string& title)
{
  if (contains_word("error 404 (not found)!!1", title)){return GOOGLE_404;}
  if (contains_word("ngnix", title)){return NGNIX;}
  if (contains_word("axis", title)){return CAMERA_AXIS_Q6055_E;}

  return "fuck";
}

int than_bruteforce(const std::string type)
{
  if (type == "n/a")
    return -1;

  if (type == CAMERA_AXIS || type == CAMERA_AXIS_205 ||
      type == CAMERA_AXIS_2400 || type == HTTP_BASIC_AUTH ||
      type == CAMERA_BB_SC384 || type == CAMERA_VB_M40 ||
      type == CAMERA_PANASONIC || type == CAMERA_UA ||
      type == CAMERA_SONY || type == CAMERA_LG_SMART ||
      type == CAMERA_EAGLE_EYE || type == CAMERA_QLIK
      || type == CAMERA_AXIS_Q6055_E)
    return HTTP_BASIC_AUTH_BRUTE;

  return -1;
}

void ftp_strategy::handle(const std::string& ip, const std::string& result, const std::string& rtt_log,
    const std::string& protocol, int port, arguments_program& argp, nesca_prints& np, NESCADATA& nd, services_nesca& sn)
{
  u8 version[2048];
  ftp_qprc_version(ip.c_str(), port, 1200, version, sizeof(version));

  if (!argp.off_ftp_brute) {
    NESCABRUTE brute(nd.brute_threads, ip.c_str(), NULL, port, nd.brute_maxcon, nd.brute_attempts, nd.brute_timeout, argp.brute_timeout_ms, FTP_BRUTEFORCE,
        nd.ftp_logins, nd.ftp_passwords, &np);
    brute_temp = brute.getres_nescastyle();
  }

  result_print = np.main_nesca_out("RES", "ftp://" + brute_temp + result, 3, "D", "", (char*)version, rtt_log, "", protocol);
  std::cout << result_print << std::endl;
}


void smtp_strategy::handle(const std::string& ip, const std::string& result, const std::string& rtt_log,
    const std::string& protocol, int port, arguments_program& argp, nesca_prints& np, NESCADATA& nd, services_nesca& sn)
{
  u8 version[2048];
  smtp_qprc_version(ip.c_str(), port, 1200, version, sizeof(version));
  result_print = np.main_nesca_out("RES", "smtp://" + brute_temp + result, 3, "D", "", (char*)version, "",rtt_log, "", protocol);
  std::cout << result_print << std::endl;
}


void hikvision_strategy::handle(const std::string& ip, const std::string& result, const std::string& rtt_log,
    const std::string& protocol, int port, arguments_program& argp, nesca_prints& np, NESCADATA& nd, services_nesca& sn)
{
#ifdef HAVE_HIKVISION
  if (!argp.off_hikvision_brute){
    NESCABRUTE brute(nd.brute_threads, ip.c_str(), NULL, port, nd.brute_maxcon, nd.brute_attempts, nd.brute_timeout, argp.brute_timeout_ms, HIKVISION_BRUTEFORCE,
        nd.hikvision_logins, nd.hikvision_passwords, &np);
    brute_temp = brute.getres_nescastyle();
  }
#endif

  result_print = np.main_nesca_out("RES", "" + brute_temp + result, 3, "", "", "", "",rtt_log, "", protocol);
  std::cout << result_print << std::endl;
}


void https_strategy::handle(const std::string& ip, const std::string& result, const std::string& rtt_log,
    const std::string& protocol, int port, arguments_program& argp, nesca_prints& np, NESCADATA& nd, services_nesca& sn)
{
  std::string result_print = np.main_nesca_out("RES", "https://" + result, 3, "", "", "", "",rtt_log, "" ,protocol);
  std::cout << result_print << std::endl;
}


void rvi_strategy::handle(const std::string& ip, const std::string& result, const std::string& rtt_log,
    const std::string& protocol, int port, arguments_program& argp, nesca_prints& np, NESCADATA& nd, services_nesca& sn)
{
#ifdef HAVE_DVR
  if (!argp.off_rvi_brute) {
    NESCABRUTE brute(nd.brute_threads, ip.c_str(), nd.get_redirect(ip).c_str(), port, nd.brute_maxcon, nd.brute_attempts, nd.brute_timeout, argp.brute_timeout_ms, RVI_BRUTEFORCE,
        nd.rvi_logins, nd.rvi_passwords, &np);
    brute_temp = brute.getres_nescastyle();
  }
#endif

  result_print = np.main_nesca_out("RES", "" + brute_temp + result, 3, "", "", "", "", rtt_log, "", protocol);

  std::cout << result_print << std::endl;
}

void rtsp_strategy::handle(const std::string& ip, const std::string& result, const std::string& rtt_log,
    const std::string& protocol, int port, arguments_program& argp, nesca_prints& np, NESCADATA& nd, services_nesca& sn)
{
  std::string pathget;

  if (!argp.off_rtsp_brute){
    for (const auto& path : rtsp_paths) {
      NESCABRUTE brute(nd.brute_threads, ip.c_str(), NULL, port, nd.brute_maxcon, nd.brute_attempts, nd.brute_timeout, argp.brute_timeout_ms, HTTP_BRUTEFORCE,
          nd.rtsp_logins, nd.rtsp_passwords, &np);
      if (!brute_temp.empty())
          pathget = path;
    }
  }

  result_print = np.main_nesca_out("RES", "rtsp://" + brute_temp + result + pathget, 3, "", "", "", "",rtt_log, "", protocol);
  std::cout << result_print << std::endl;
}

#define HTTP_BUFFER_SIZE 655354
void http_strategy::handle(const std::string& ip, const std::string& result, const std::string& rtt_log,
    const std::string& protocol, int port, arguments_program& argp, nesca_prints& np, NESCADATA& nd, services_nesca& sn)
{
  int brute, robots_txt, sitemap;
  std::string httpcheck, httpcheck1, httpcheck2, redirect, htmlpro;
  char title[HTTP_BUFFER_SIZE];

  htmlpro = nd.get_html(ip);
  http_qprc_title(htmlpro.c_str(), title, HTTP_BUFFER_SIZE);
  redirect = nd.get_redirect(ip);

  http_title = title;
  if (http_title.empty())
    http_title = "n/a";

  for (const auto& n : argp.nesca_negatives) {
    const std::string& first = n.first;
    const std::string& second = n.second;

    if (second == "1" || second == "title") {
      if (contains_word(first, title)) {
        if (argp.debug)
          np.nlog_custom("WARNING", "Skip negative: "+first+"\n", 2);
        return;
      }
    }
    else if (second == "2" || second == "code" || second == "header") {
      if (contains_word(first, htmlpro.c_str())) {
        if (argp.debug)
          np.nlog_custom("WARNING", "Skip negative: "+first+"\n", 2);
        return;
      }
    }
    else if (second == "3" || second == "path" || second == "redirect") {
      if (contains_word(first, redirect)) {
        if (argp.debug)
          np.nlog_custom("WARNING", "Skip negative: "+first+"\n", 2);
        return;
      }
    }
  }

  type_target = "n/a";
  httpcheck = set_target_at_path(redirect);
  httpcheck1 = set_target_at_http_header(htmlpro.c_str());
  httpcheck2 = set_target_at_title(title);

  if (httpcheck != "fuck")
    type_target = httpcheck;
  if (httpcheck1 != "fuck")
    type_target = httpcheck1;
  if (httpcheck2 != "fuck")
    type_target = httpcheck2;

  brute = than_bruteforce(type_target);

  if (!argp.off_http_brute && type_target != "fuck" && brute != EOF) {
    NESCABRUTE brute(nd.brute_threads, ip.c_str(), nd.get_redirect(ip).c_str(), port, nd.brute_maxcon, nd.brute_attempts, nd.brute_timeout, argp.brute_timeout_ms, HTTP_BRUTEFORCE,
        nd.http_logins, nd.http_passwords, &np);
    brute_temp = brute.getres_nescastyle();
  }

  result_print = np.main_nesca_out("RES", "http://" + brute_temp + ip + ":" + std::to_string(port),
      3, "T", "D", http_title, type_target, rtt_log, "", protocol);

  std::cout << result_print << std::endl;

  if (!redirect.empty())
    np.nlog_redirect(redirect);
  if (argp.get_response) {
    np.yellow_html_on();
    std::cout << htmlpro << std::endl;
    reset_colors;
  }

  if (argp.robots_txt){
    np.gray_nesca_on();
    std::cout << "  * ROBOTS ";
    reset_colors;

    robots_txt = http_qprc_robots_txt(ip.c_str(), port, 1100);

    if (robots_txt == -1) {
      np.red_html_on();
      std::cout << ip << " robots.txt not found!\n";
      reset_colors;
    }
    else {
      np.green_html_on();
      std::cout << "http://" << ip + "/robots.txt\n";
      reset_colors;
    }
  }

  if (argp.sitemap_xml){
    np.gray_nesca_on();
    std::cout << "  * STEMAP ";
    reset_colors;

    sitemap = http_qprc_sitemap_xml(ip.c_str(), port, 1100);
    if (sitemap == -1) {
      np.red_html_on();
      std::cout << ip << " sitemap.xml not found!\n";
      reset_colors;
    }
    else {
      np.green_html_on();
      std::cout << "http://" << ip + "/sitemap.xml\n";
      reset_colors;
    }
  }

  if (argp.find) {
    for (const auto& target : argp.find_target){
      std::vector<std::string> finds = find_sentences_with_word(target, htmlpro);
      if (!finds.empty()) {
        for (const auto& find : finds){
          np.gray_nesca_on();
          std::cout << "  * FOUND ";
          np.green_html_on();
          std::cout << find;
          reset_colors;
          np.gray_nesca_on();
          std::cout << " example: ";
          reset_colors;
          np.golder_rod_on();
          std::cout << "\"" << target << "\"" << std::endl;
          reset_colors;
        }
      }
    }
  }
}

void else_strategy::handle(const std::string& ip, const std::string& result, const std::string& rtt_log,
    const std::string& protocol, int port, arguments_program& argp, nesca_prints& np, NESCADATA& nd, services_nesca& sn)
{
  std::cout << np.main_nesca_out("RES", result, 3, "", "", "", "",rtt_log, "", protocol) << std::endl;
}
