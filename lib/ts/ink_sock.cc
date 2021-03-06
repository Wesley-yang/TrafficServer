/** @file

  A brief file description

  @section license License

  Licensed to the Apache Software Foundation (ASF) under one
  or more contributor license agreements.  See the NOTICE file
  distributed with this work for additional information
  regarding copyright ownership.  The ASF licenses this file
  to you under the Apache License, Version 2.0 (the
  "License"); you may not use this file except in compliance
  with the License.  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 */

/***************************************************************************
  Socket operations



***************************************************************************/
#include "libts.h"

#include "ink_unused.h"      /* MAGIC_EDITING_TAG */

//
// Compilation options
//

// #define CHECK_PLAUSIBILITY_OF_SOCKADDR


#ifdef CHECK_PLAUSIBILITY_OF_SOCKADDR
#define CHECK_PLAUSIBLE_SOCKADDR(_n,_f,_l) check_plausible_sockaddr(_n,_f,_n)
inline void
check_valid_sockaddr(sockaddr * sa, char *file, int line)
{
  sockaddr_in *si = (sockaddr_in *) sa;
  unsigned short port = ntohs(si->sin_port);
  unsigned short addr = ntohl(si->sin_addr.s_addr);

  if (port > 20000) {
    cerr << "[byteordering] In " << file << ", line " << line << " the IP port ";
    cerr << "was found to be " << port << "(in host byte order).\n";
    cerr << "[byteordering] This seems inplausible, so check for byte order problems\n";
  }
}
#else
#define CHECK_PLAUSIBLE_SOCKADDR(_n,_f,_l)
#endif

int
safe_setsockopt(int s, int level, int optname, char *optval, int optlevel)
{
  int r;
  do {
    r = setsockopt(s, level, optname, optval, optlevel);
  } while (r < 0 && (errno == EAGAIN || errno == EINTR));
  return r;
}

int
safe_getsockopt(int s, int level, int optname, char *optval, int *optlevel)
{
  int r;
  do {
    r = getsockopt(s, level, optname, optval, (socklen_t *) optlevel);
  } while (r < 0 && (errno == EAGAIN || errno == EINTR));
  return r;
}

int
safe_fcntl(int fd, int cmd, int arg)
{
  int r = 0;
  do {
    r = fcntl(fd, cmd, arg);
  } while (r < 0 && (errno == EAGAIN || errno == EINTR));
  return r;
}

int
safe_set_fl(int fd, int arg)
{
  int flags = safe_fcntl(fd, F_GETFL, 0);
  if (flags < 0)
    return flags;
  flags |= arg;
  flags = safe_fcntl(fd, F_SETFL, flags);
  return flags;
}

int
safe_clr_fl(int fd, int arg)
{
  int flags = safe_fcntl(fd, F_GETFL, 0);
  if (flags < 0)
    return flags;
  flags &= ~arg;
  flags = safe_fcntl(fd, F_SETFL, flags);
  return flags;
}

int
safe_nonblocking(int fd)
{
  return safe_set_fl(fd, O_NONBLOCK);
}

int
safe_blocking(int fd)
{
  return safe_clr_fl(fd, O_NONBLOCK);
}

int
write_ready(int fd)
{
  struct pollfd p;
  p.events = POLLOUT;
  p.fd = fd;
  int r = poll(&p, 1, 0);
  if (r <= 0)
    return r;
  if (p.revents & (POLLERR | POLLNVAL))
    return -1;
  if (p.revents & (POLLOUT | POLLHUP))
    return 1;
  return 0;
}

int
read_ready(int fd)
{
  struct pollfd p;
  p.events = POLLIN;
  p.fd = fd;
  int r = poll(&p, 1, 0);
  if (r <= 0)
    return r;
  if (p.revents & (POLLERR | POLLNVAL))
    return -1;
  if (p.revents & (POLLIN | POLLHUP))
    return 1;
  return 0;
}

int
safe_ioctl(int fd, int request, char *arg)
{
  int r = -1;
  do {
    r = ioctl(fd, request, arg);
  } while (r < 0 && (errno == EAGAIN || errno == EINTR));
  return r;
}

int
safe_bind(int s, struct sockaddr const* name, int namelen)
{
  int r;
  CHECK_PLAUSIBLE_SOCKADDR(name, __FILE__, __LINE__);
  do {
    r = bind(s, name, namelen);
  } while (r < 0 && (errno == EAGAIN || errno == EINTR));
  return r;
}

int
safe_listen(int s, int backlog)
{
  int r;
  do {
    r = listen(s, backlog);
  } while (r < 0 && (errno == EAGAIN || errno == EINTR));
  return r;
}

int
safe_getsockname(int s, struct sockaddr *name, int *namelen)
{
  int r;
  do {
    r = getsockname(s, name, (socklen_t *) namelen);
  } while (r < 0 && (errno == EAGAIN || errno == EINTR));
  return r;
}

char
fd_read_char(int fd)
{
  char c;
  int r;
  do {
    r = read(fd, &c, 1);
    if (r > 0)
      return c;
  } while (r < 0 && (errno == EAGAIN || errno == EINTR));
  perror("fd_read_char");
  ink_assert(!"fd_read_char");
  return c;
}

int
fd_read_line(int fd, char *s, int len)
{
  char c;
  int numread = 0, r;
  //char *buf = s;
  do {
    do
      r = read(fd, &c, 1);
    while (r < 0 && (errno == EAGAIN || errno == EINTR));

    if (r <= 0 && numread)
      break;

    if (r <= 0)
      return r;

    if (c == '\n')
      break;

    s[numread++] = c;
  } while (numread < len - 1);

  s[numread] = 0;
  return numread;
}


int
close_socket(int s)
{
  return close(s);
}

int
write_socket(int s, const char *buffer, int length)
{
  return write(s, (const void *) buffer, length);
}

int
read_socket(int s, char *buffer, int length)
{
  return read(s, (void *) buffer, length);
}
