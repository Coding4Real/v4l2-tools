/*
 * Copyright (c) 2013 - Mauro Carvalho Chehab <m.chehab@samsung.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 * Or, point your browser to http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 *
 * Based on ETSI EN 300 468 V1.11.1 (2010-04)
 */

#include <linux/videodev2.h>

#include "descriptors.h"
#include "descriptors/desc_extension.h"
#include "descriptors/desc_t2_delivery.h"
#include "dvb-fe.h"

void dvb_desc_t2_delivery_init(struct dvb_v5_fe_parms *parms,
			       const uint8_t *buf,
			       struct dvb_extension_descriptor *ext,
			       void *desc)
{
	struct dvb_desc_t2_delivery *d = desc;
	unsigned char *p = (unsigned char *) buf;
	size_t desc_len = ext->length - 1, len, len2;
	int i;

	len = offsetof(struct dvb_desc_t2_delivery, bitfield);
	len2 = offsetof(struct dvb_desc_t2_delivery, centre_frequency);

	if (desc_len < len) {
		dvb_logwarn("T2 delivery descriptor is too small");
		return;
	}
	if (desc_len < len2) {
		memcpy(p, buf, len);
		bswap16(d->system_id);

		if (desc_len != len)
			dvb_logwarn("T2 delivery descriptor is truncated");

		return;
	}
	memcpy(p, buf, len2);
	p += len2;

	len = desc_len - (p - buf);
	memcpy(&d->centre_frequency, p, len);
	p += len;

	if (d->tfs_flag)
		d->frequency_loop_length = 1;
	else {
		d->frequency_loop_length = *p;
		p++;
	}

	d->centre_frequency = calloc(d->frequency_loop_length,
				     sizeof(*d->centre_frequency));
	if (!d->centre_frequency) {
		dvb_perror("Out of memory");
		return;
	}

	memcpy(d->centre_frequency, p, sizeof(*d->centre_frequency) * d->frequency_loop_length);
	p += sizeof(*d->centre_frequency) * d->frequency_loop_length;

	for (i = 0; i < d->frequency_loop_length; i++)
		bswap32(d->centre_frequency[i]);

	d->subcel_info_loop_length = *p;
	p++;

	d->subcell = calloc(d->subcel_info_loop_length, sizeof(*d->subcell));
	if (!d->subcell) {
		dvb_perror("Out of memory");
		return;
	}
	memcpy(d->subcell, p, sizeof(*d->subcell) * d->subcel_info_loop_length);

	for (i = 0; i < d->subcel_info_loop_length; i++)
		bswap16(d->subcell[i].transposer_frequency);
}

void dvb_desc_t2_delivery_print(struct dvb_v5_fe_parms *parms,
				const struct dvb_extension_descriptor *ext,
				const void *desc)
{
	const struct dvb_desc_t2_delivery *d = desc;
	int i;

	dvb_log("|       DVB-T2 delivery");
	dvb_log("|           plp_id                    %d", d->plp_id);
	dvb_log("|           system_id                 %d", d->system_id);

	if (ext->length - 1 <= 4)
		return;

	dvb_log("|           tfs_flag                  %d", d->tfs_flag);
	dvb_log("|           other_frequency_flag      %d", d->other_frequency_flag);
	dvb_log("|           transmission_mode         %d", d->transmission_mode);
	dvb_log("|           guard_interval            %d", d->guard_interval);
	dvb_log("|           reserved                  %d", d->reserved);
	dvb_log("|           bandwidth                 %d", d->bandwidth);
	dvb_log("|           SISO MISO                 %d", d->SISO_MISO);

	for (i = 0; i < d->frequency_loop_length; i++)
		dvb_log("|           centre frequency[%d]   %d", i, d->centre_frequency[i]);

	for (i = 0; i < d->subcel_info_loop_length; i++) {
		dvb_log("|           cell_id_extension[%d]  %d", i, d->subcell[i].cell_id_extension);
		dvb_log("|           transposer frequency   %d", d->subcell[i].transposer_frequency);
	}
}

void dvb_desc_t2_delivery_free(const void *desc)
{
	const struct dvb_desc_t2_delivery *d = desc;

	if (d->centre_frequency)
		free(d->centre_frequency);

	if (d->subcell)
		free(d->subcell);
}

const unsigned dvbt2_bw[] = {
	[0] =  8000000,
	[1] =  7000000,
	[2] =  6000000,
	[3] =  5000000,
	[4] = 10000000,
	[5] =  1712000,
	[6 ...15] = 0,		/* Reserved */
};
const uint32_t dvbt2_interval[] = {
	[0] = GUARD_INTERVAL_1_32,
	[1] = GUARD_INTERVAL_1_16,
	[2] = GUARD_INTERVAL_1_8,
	[3] = GUARD_INTERVAL_1_4,
	[4] = GUARD_INTERVAL_1_128,
	[5] = GUARD_INTERVAL_19_128,
	[6] = GUARD_INTERVAL_19_256,
	[7 ...15] = GUARD_INTERVAL_AUTO /* Reserved */
};
const unsigned dvbt2_transmission_mode[] = {
	[0] = TRANSMISSION_MODE_2K,
	[1] = TRANSMISSION_MODE_8K,
	[2] = TRANSMISSION_MODE_4K,
	[3] = TRANSMISSION_MODE_1K,
	[4] = TRANSMISSION_MODE_16K,
	[5] = TRANSMISSION_MODE_32K,
	[6 ...7] = TRANSMISSION_MODE_AUTO,	/* Reserved */
};
