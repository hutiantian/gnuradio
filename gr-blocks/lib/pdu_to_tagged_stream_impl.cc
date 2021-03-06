/* -*- c++ -*- */
/*
 * Copyright 2013 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pdu_to_tagged_stream_impl.h"
#include <blocks/pdu.h>
#include <gr_io_signature.h>

namespace gr {
  namespace blocks {

    pdu_to_tagged_stream::sptr
    pdu_to_tagged_stream::make(pdu::vector_type type)
    {
      return gnuradio::get_initial_sptr(new pdu_to_tagged_stream_impl(type));
    }

    pdu_to_tagged_stream_impl::pdu_to_tagged_stream_impl(pdu::vector_type type)
      : gr_sync_block("pdu_to_tagged_stream",
		      gr_make_io_signature(0, 0, 0),
		      gr_make_io_signature(1, 1, pdu::itemsize(type))),
	d_itemsize(pdu::itemsize(type)),
	d_type(type)
    {
      message_port_register_in(PDU_PORT_ID);
    }

    int
    pdu_to_tagged_stream_impl::work(int noutput_items,
				    gr_vector_const_void_star &input_items,
				    gr_vector_void_star &output_items)
    {   
      char *out = (char *)output_items[0];
      int nout = 0;

      // if we have remaining output, send it
      if (d_remain.size() > 0) {
	nout = std::min((size_t)d_remain.size()/d_itemsize, (size_t)noutput_items);
	memcpy(out, &d_remain[0], nout*d_itemsize);
	d_remain.erase(d_remain.begin(), d_remain.begin()+nout);
	noutput_items -= nout;
	out += nout*d_itemsize;
      }

      // if we have space for at least one item output as much as we can
      if (noutput_items > 0) {

	// grab a message if one exists
	pmt::pmt_t msg(delete_head_blocking(PDU_PORT_ID));
	if (msg.get() == NULL)
	  return nout;

	// make sure type is valid
	if (!pmt::pmt_is_pair(msg)) // TODO: implement pdu::is_valid()
	  throw std::runtime_error("received a malformed pdu message");
	
	// grab the components of the pdu message
	pmt::pmt_t meta(pmt::pmt_car(msg));
	pmt::pmt_t vect(pmt::pmt_cdr(msg));

	// compute offset for output tag
	uint64_t offset = nitems_written(0) + nout;

	// add a tag for pdu length
	add_item_tag(0, offset, PDU_LENGTH_TAG, pmt::pmt_from_long(pmt::pmt_length(vect)), pmt::mp(alias()));

	// if we recieved metadata add it as tags
	if (!pmt_eq(meta, pmt::PMT_NIL) ) {
	  pmt::pmt_t pair(pmt::pmt_dict_keys(meta));

	  while (!pmt_eq(pair, pmt::PMT_NIL) ) {
            pmt::pmt_t k(pmt::pmt_cdr(pair));
            pmt::pmt_t v(pmt::pmt_dict_ref(meta, k, pmt::PMT_NIL));
            add_item_tag(0, offset, k, v, pmt::mp(alias()));
            }
        }

	// copy vector output
	size_t ncopy = std::min((size_t)noutput_items, (size_t)pmt::pmt_length(vect));
	size_t nsave = pmt::pmt_length(vect) - ncopy;
 
	// copy output
	size_t io(0);
	nout += ncopy;
	memcpy(out, pmt_uniform_vector_elements(vect,io), ncopy*d_itemsize);
	
	// save leftover items if needed for next work call
	if (nsave > 0) {
	  d_remain.resize(nsave*d_itemsize, 0);
	  memcpy(&d_remain[0], pmt_uniform_vector_elements(vect,ncopy), nsave*d_itemsize);
        }
      }
      
      return nout;
    }

  } /* namespace blocks */
} /* namespace gr */
