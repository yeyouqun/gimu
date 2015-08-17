/* -*- C++ -*- */
// SUN_Proactor.inl,v 4.1 2004/06/16 07:57:23 jwillemsen Exp

ACE_INLINE 
ACE_POSIX_Proactor::Proactor_Type ACE_SUN_Proactor::get_impl_type (void)
{
  return PROACTOR_SUN;
} 
