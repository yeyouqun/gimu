/* -*- C++ -*- */
// POSIX_CB_Proactor.inl,v 4.1 2004/06/16 07:57:23 jwillemsen Exp

#if defined (ACE_HAS_AIO_CALLS)

ACE_INLINE
ACE_POSIX_Proactor::Proactor_Type ACE_POSIX_CB_Proactor::get_impl_type (void)
{
  return PROACTOR_CB;
}

#endif /* ACE_HAS_AIO_CALLS */
