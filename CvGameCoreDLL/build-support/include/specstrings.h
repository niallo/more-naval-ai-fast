#ifndef SPECSTRINGS_H
#define SPECSTRINGS_H

#include <sal.h>

#ifndef __xcount
#define __xcount(size)
#endif
#ifndef __xcount_opt
#define __xcount_opt(size)
#endif

#ifndef __in_xcount
#define __in_xcount(size)
#endif
#ifndef __out_xcount
#define __out_xcount(size)
#endif
#ifndef __inout_xcount
#define __inout_xcount(size)
#endif
#ifndef __in_xcount_opt
#define __in_xcount_opt(size)
#endif
#ifndef __out_xcount_opt
#define __out_xcount_opt(size)
#endif
#ifndef __inout_xcount_opt
#define __inout_xcount_opt(size)
#endif

#ifndef __out_xcount_part
#define __out_xcount_part(size, length)
#endif
#ifndef __in_awcount
#define __in_awcount(expr, size)
#endif
#ifndef __out_awcount
#define __out_awcount(expr, size)
#endif
#ifndef __inout_xcount_part
#define __inout_xcount_part(size, length)
#endif
#ifndef __out_xcount_full
#define __out_xcount_full(size)
#endif
#ifndef __inout_xcount_full
#define __inout_xcount_full(size)
#endif
#ifndef __out_xcount_part_opt
#define __out_xcount_part_opt(size, length)
#endif
#ifndef __inout_xcount_part_opt
#define __inout_xcount_part_opt(size, length)
#endif
#ifndef __out_xcount_full_opt
#define __out_xcount_full_opt(size)
#endif
#ifndef __inout_xcount_full_opt
#define __inout_xcount_full_opt(size)
#endif

#ifndef __deref_in
#define __deref_in
#endif
#ifndef __deref_in_opt
#define __deref_in_opt
#endif
#ifndef __deref_xcount
#define __deref_xcount(size)
#endif
#ifndef __deref_xcount_opt
#define __deref_xcount_opt(size)
#endif
#ifndef __deref_opt_xcount
#define __deref_opt_xcount(size)
#endif
#ifndef __deref_opt_xcount_opt
#define __deref_opt_xcount_opt(size)
#endif

#ifndef __deref_in_ecount
#define __deref_in_ecount(size)
#endif
#ifndef __deref_in_bcount
#define __deref_in_bcount(size)
#endif
#ifndef __deref_in_xcount
#define __deref_in_xcount(size)
#endif
#ifndef __deref_in_ecount_opt
#define __deref_in_ecount_opt(size)
#endif
#ifndef __deref_in_bcount_opt
#define __deref_in_bcount_opt(size)
#endif
#ifndef __deref_in_xcount_opt
#define __deref_in_xcount_opt(size)
#endif

#ifndef __deref_in_ecount_z
#define __deref_in_ecount_z(size)
#endif
#ifndef __deref_in_bcount_z
#define __deref_in_bcount_z(size)
#endif
#ifndef __deref_in_ecount_z_opt
#define __deref_in_ecount_z_opt(size)
#endif
#ifndef __deref_in_bcount_z_opt
#define __deref_in_bcount_z_opt(size)
#endif

#ifndef __deref_out_xcount
#define __deref_out_xcount(size)
#endif
#ifndef __deref_out_xcount_opt
#define __deref_out_xcount_opt(size)
#endif
#ifndef __deref_out_xcount_part
#define __deref_out_xcount_part(size, length)
#endif
#ifndef __deref_out_xcount_full
#define __deref_out_xcount_full(size)
#endif
#ifndef __deref_out_xcount_part_opt
#define __deref_out_xcount_part_opt(size, length)
#endif
#ifndef __deref_out_xcount_full_opt
#define __deref_out_xcount_full_opt(size)
#endif

#ifndef __deref_out_ecount_z
#define __deref_out_ecount_z(size)
#endif
#ifndef __deref_out_bcount_z
#define __deref_out_bcount_z(size)
#endif
#ifndef __deref_out_xcount_z
#define __deref_out_xcount_z(size)
#endif
#ifndef __deref_out_ecount_z_opt
#define __deref_out_ecount_z_opt(size)
#endif
#ifndef __deref_out_bcount_z_opt
#define __deref_out_bcount_z_opt(size)
#endif
#ifndef __deref_out_xcount_z_opt
#define __deref_out_xcount_z_opt(size)
#endif

#ifndef __deref_inout_xcount
#define __deref_inout_xcount(size)
#endif
#ifndef __deref_inout_xcount_opt
#define __deref_inout_xcount_opt(size)
#endif
#ifndef __deref_inout_xcount_part
#define __deref_inout_xcount_part(size, length)
#endif
#ifndef __deref_inout_xcount_full
#define __deref_inout_xcount_full(size)
#endif
#ifndef __deref_inout_xcount_part_opt
#define __deref_inout_xcount_part_opt(size, length)
#endif
#ifndef __deref_inout_xcount_full_opt
#define __deref_inout_xcount_full_opt(size)
#endif

#ifndef __deref_inout_ecount_z
#define __deref_inout_ecount_z(size)
#endif
#ifndef __deref_inout_bcount_z
#define __deref_inout_bcount_z(size)
#endif
#ifndef __deref_inout_xcount_z
#define __deref_inout_xcount_z(size)
#endif
#ifndef __deref_inout_ecount_z_opt
#define __deref_inout_ecount_z_opt(size)
#endif
#ifndef __deref_inout_bcount_z_opt
#define __deref_inout_bcount_z_opt(size)
#endif
#ifndef __deref_inout_xcount_z_opt
#define __deref_inout_xcount_z_opt(size)
#endif

#ifndef __deref_opt_in
#define __deref_opt_in
#endif
#ifndef __deref_opt_in_opt
#define __deref_opt_in_opt
#endif
#ifndef __deref_opt_in_ecount
#define __deref_opt_in_ecount(size)
#endif
#ifndef __deref_opt_in_bcount
#define __deref_opt_in_bcount(size)
#endif
#ifndef __deref_opt_in_xcount
#define __deref_opt_in_xcount(size)
#endif
#ifndef __deref_opt_in_ecount_opt
#define __deref_opt_in_ecount_opt(size)
#endif
#ifndef __deref_opt_in_bcount_opt
#define __deref_opt_in_bcount_opt(size)
#endif
#ifndef __deref_opt_in_xcount_opt
#define __deref_opt_in_xcount_opt(size)
#endif

#ifndef __field_xcount
#define __field_xcount(size)
#endif
#ifndef __field_ecount
#define __field_ecount(size)
#endif
#ifndef __field_bcount
#define __field_bcount(size)
#endif
#ifndef __field_xcount_opt
#define __field_xcount_opt(size)
#endif
#ifndef __field_ecount_opt
#define __field_ecount_opt(size)
#endif
#ifndef __field_bcount_opt
#define __field_bcount_opt(size)
#endif
#ifndef __field_xcount_part
#define __field_xcount_part(size, init)
#endif
#ifndef __field_ecount_part
#define __field_ecount_part(size, init)
#endif
#ifndef __field_bcount_part
#define __field_bcount_part(size, init)
#endif
#ifndef __field_xcount_part_opt
#define __field_xcount_part_opt(size, init)
#endif
#ifndef __field_ecount_part_opt
#define __field_ecount_part_opt(size, init)
#endif
#ifndef __field_bcount_part_opt
#define __field_bcount_part_opt(size, init)
#endif
#ifndef __field_xcount_full
#define __field_xcount_full(size)
#endif
#ifndef __field_ecount_full
#define __field_ecount_full(size)
#endif
#ifndef __field_bcount_full
#define __field_bcount_full(size)
#endif
#ifndef __field_xcount_full_opt
#define __field_xcount_full_opt(size)
#endif
#ifndef __field_ecount_full_opt
#define __field_ecount_full_opt(size)
#endif
#ifndef __field_bcount_full_opt
#define __field_bcount_full_opt(size)
#endif

#ifndef __struct_bcount
#define __struct_bcount(size)
#endif
#ifndef __struct_xcount
#define __struct_xcount(size)
#endif

#ifndef __RPC__in
#define __RPC__in
#endif
#ifndef __RPC__out
#define __RPC__out
#endif
#ifndef __RPC__inout
#define __RPC__inout
#endif
#ifndef __RPC__in_opt
#define __RPC__in_opt
#endif
#ifndef __RPC__out_opt
#define __RPC__out_opt
#endif
#ifndef __RPC__inout_opt
#define __RPC__inout_opt
#endif

#endif
