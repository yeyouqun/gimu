// -*- C++ -*-
//
// Ping_Socket.inl,v 1.1 2004/09/25 05:42:02 ossama Exp


ACE_INLINE char *
ACE::Ping_Socket::icmp_recv_buff (void)
{
  return this->icmp_recv_buff_;
}
