/*
 * Copyright 2011 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_GR_TAGS_H
#define INCLUDED_GR_TAGS_H

#include <gr_runtime_api.h>
#include <pmt/pmt.h>

struct GR_RUNTIME_API gr_tag_t{

    //! the item \p tag occurred at (as a uint64_t)
    uint64_t offset;

    //! the key of \p tag (as a PMT symbol)
    pmt::pmt_t key;

    //! the value of \p tag (as a PMT)
    pmt::pmt_t value;

    //! the source ID of \p tag (as a PMT)
    pmt::pmt_t srcid;

    //! Used by gr_buffer to mark a tagged as deleted by a specific block. You can usually ignore this.
    std::vector<long> marked_deleted;

    //! Comparison function to test which tag, \p x or \p y, came first in time
    static inline bool offset_compare(
        const gr_tag_t &x, const gr_tag_t &y
    ){
        return x.offset < y.offset;
    }

    inline bool operator == (const gr_tag_t &t) const
    {
      return (t.key == key) && (t.value == value) && (t.srcid == srcid) && (t.offset == offset);
    }
};

#endif /*INCLUDED_GR_TAGS_H*/