#ifndef CIV4SDK_COMPAT_SAL_H
#define CIV4SDK_COMPAT_SAL_H

/*
 * The Windows SDK archive used for this Wine build includes specstrings.h but
 * not sal.h. VC++ 7.1 does not consume SAL annotations, so define them away.
 */
#define __SAL_H_FULL_VER 140050727

#ifndef SPECSTRINGIZE
#define SPECSTRINGIZE(x) #x
#endif

#define __null
#define __notnull
#define __maybenull
#define __readonly
#define __notreadonly
#define __valid
#define __notvalid
#define __pre
#define __post
#define __deref
#define __exceptthat
#define __refparam
#define __inner_checkReturn
#define __inner_control_entrypoint(kind)
#define __nothrow
#define __nullterminated
#define __nullnullterminated
#define __possibly_notnullterminated

#define __readableTo(size)
#define __writableTo(size)
#define __elem_readableTo(size)
#define __elem_writableTo(size)
#define __byte_readableTo(size)
#define __byte_writableTo(size)

#define __ecount(size)
#define __bcount(size)
#define __ecount_opt(size)
#define __bcount_opt(size)

#define __in
#define __out
#define __inout
#define __in_opt
#define __out_opt
#define __inout_opt

#define __in_ecount(size)
#define __in_bcount(size)
#define __out_ecount(size)
#define __out_bcount(size)
#define __inout_ecount(size)
#define __inout_bcount(size)

#define __in_ecount_opt(size)
#define __in_bcount_opt(size)
#define __out_ecount_opt(size)
#define __out_bcount_opt(size)
#define __inout_ecount_opt(size)
#define __inout_bcount_opt(size)

#define __out_ecount_part(size, length)
#define __out_bcount_part(size, length)
#define __in_awcount(expr, size)
#define __out_awcount(expr, size)
#define __inout_ecount_part(size, length)
#define __inout_bcount_part(size, length)

#define __out_ecount_full(size)
#define __out_bcount_full(size)
#define __inout_ecount_full(size)
#define __inout_bcount_full(size)

#define __out_ecount_part_opt(size, length)
#define __out_bcount_part_opt(size, length)
#define __inout_ecount_part_opt(size, length)
#define __inout_bcount_part_opt(size, length)

#define __out_ecount_full_opt(size)
#define __out_bcount_full_opt(size)
#define __inout_ecount_full_opt(size)
#define __inout_bcount_full_opt(size)

#define __in_z
#define __out_z
#define __inout_z
#define __in_z_opt
#define __out_z_opt
#define __inout_z_opt

#define __in_ecount_z(size)
#define __in_bcount_z(size)
#define __out_ecount_z(size)
#define __out_bcount_z(size)
#define __inout_ecount_z(size)
#define __inout_bcount_z(size)

#define __in_ecount_z_opt(size)
#define __in_bcount_z_opt(size)
#define __out_ecount_z_opt(size)
#define __out_bcount_z_opt(size)
#define __inout_ecount_z_opt(size)
#define __inout_bcount_z_opt(size)

#define __deref_out
#define __deref_inout
#define __deref_opt_out
#define __deref_opt_inout
#define __deref_out_opt
#define __deref_inout_opt
#define __deref_opt_out_opt
#define __deref_opt_inout_opt

#define __deref_ecount(size)
#define __deref_bcount(size)
#define __deref_out_ecount(size)
#define __deref_out_bcount(size)
#define __deref_inout_ecount(size)
#define __deref_inout_bcount(size)
#define __deref_opt_out_ecount(size)
#define __deref_opt_out_bcount(size)
#define __deref_opt_inout_ecount(size)
#define __deref_opt_inout_bcount(size)

#define __deref_ecount_opt(size)
#define __deref_bcount_opt(size)
#define __deref_out_ecount_opt(size)
#define __deref_out_bcount_opt(size)
#define __deref_inout_ecount_opt(size)
#define __deref_inout_bcount_opt(size)
#define __deref_opt_out_ecount_opt(size)
#define __deref_opt_out_bcount_opt(size)
#define __deref_opt_inout_ecount_opt(size)
#define __deref_opt_inout_bcount_opt(size)

#define __deref_out_ecount_part(size, length)
#define __deref_out_bcount_part(size, length)
#define __deref_inout_ecount_part(size, length)
#define __deref_inout_bcount_part(size, length)
#define __deref_opt_out_ecount_part(size, length)
#define __deref_opt_out_bcount_part(size, length)
#define __deref_opt_inout_ecount_part(size, length)
#define __deref_opt_inout_bcount_part(size, length)

#define __deref_out_ecount_full(size)
#define __deref_out_bcount_full(size)
#define __deref_inout_ecount_full(size)
#define __deref_inout_bcount_full(size)
#define __deref_opt_out_ecount_full(size)
#define __deref_opt_out_bcount_full(size)
#define __deref_opt_inout_ecount_full(size)
#define __deref_opt_inout_bcount_full(size)

#define __deref_out_z
#define __deref_inout_z
#define __deref_opt_out_z
#define __deref_opt_inout_z
#define __deref_out_z_opt
#define __deref_inout_z_opt
#define __deref_opt_out_z_opt
#define __deref_opt_inout_z_opt
#define __deref_out_ecount_z(size)
#define __deref_out_bcount_z(size)
#define __deref_inout_ecount_z(size)
#define __deref_inout_bcount_z(size)
#define __deref_opt_out_ecount_z(size)
#define __deref_opt_out_bcount_z(size)
#define __deref_opt_inout_ecount_z(size)
#define __deref_opt_inout_bcount_z(size)
#define __deref_out_ecount_z_opt(size)
#define __deref_out_bcount_z_opt(size)
#define __deref_inout_ecount_z_opt(size)
#define __deref_inout_bcount_z_opt(size)
#define __deref_opt_out_ecount_z_opt(size)
#define __deref_opt_out_bcount_z_opt(size)
#define __deref_opt_inout_ecount_z_opt(size)
#define __deref_opt_inout_bcount_z_opt(size)

#define __reserved
#define __checkReturn
#define __callback
#define __format_string
#define __fallthrough
#define __success(expr)
#define __typefix(ctype)
#define __override
#define __analysis_assume(expr)
#define __analysis_assert(expr)
#define __analysis_hint(hint)
#define __assume_bound(value)
#define __assume_validated(value)
#define __allocator
#define __bound
#define __range(lb, ub)
#define __in_bound
#define __out_bound
#define __deref_out_bound
#define __in_range(lb, ub)
#define __out_range(lb, ub)
#define __deref_in_range(lb, ub)
#define __deref_out_range(lb, ub)
#define __field_range(lb, ub)
#define __field_data_source(src_sym)
#define __in_data_source(src_sym)
#define __out_data_source(src_sym)
#define __out_validated(typ_sym)
#define __this_out_data_source(src_sym)
#define __this_out_validated(typ_sym)
#define __transfer(formal)
#define __rpc_entry
#define __kernel_entry
#define __gdi_entry
#define __encoded_pointer
#define __encoded_array
#define __field_encoded_pointer
#define __field_encoded_array
#define __type_has_adt_prop(adt, prop)
#define __out_has_adt_prop(adt, prop)
#define __out_not_has_adt_prop(adt, prop)
#define __out_transfer_adt_prop(arg)
#define __out_has_type_adt_props(typ)
#define __volatile
#define __nonvolatile
#define __deref_volatile
#define __deref_nonvolatile

#define __field_ecount(size)
#define __field_bcount(size)
#define __field_ecount_opt(size)
#define __field_bcount_opt(size)
#define __field_ecount_part(size, init)
#define __field_bcount_part(size, init)
#define __field_ecount_part_opt(size, init)
#define __field_bcount_part_opt(size, init)
#define __field_ecount_full(size)
#define __field_bcount_full(size)
#define __field_ecount_full_opt(size)
#define __field_bcount_full_opt(size)

#endif
