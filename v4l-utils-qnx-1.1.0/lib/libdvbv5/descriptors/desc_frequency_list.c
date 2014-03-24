/*
 * Copyright (c) 2011-2012 - Mauro Carvalho Chehab
 * Copyright (c) 2012 - Andre Roth <neolynx@gmail.com>
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
 */

#include <linux/videodev2.h>

#include "descriptors/desc_frequency_list.h"
#include "descriptors.h"
#include "dvb-fe.h"

void dvb_desc_frequency_list_init(struct dvb_v5_fe_parms *parms, const uint8_t *buf, struct dvb_desc *desc)
{
	struct dvb_desc_frequency_list *d = (struct dvb_desc_frequency_list *) desc;
	size_t len;
	int i;
	const uint8_t *p = buf;

	len = sizeof(*d) - offsetof(struct dvb_desc_frequency_list, bitfield);
	memcpy(&d->bitfield, p, len);
	p += len;

	d->frequencies = (d->length - len) / sizeof(d->frequency[0]);

	d->frequency = calloc(1, sizeof(d->frequency));

	for (i = 0; i < d->frequencies; i++) {
		d->frequency[i] = ((uint32_t *) buf)[i];
		bswap32(d->frequency[i]);
		switch (d->freq_type) {
			case 1: /* satellite - to get kHz */
			case 3: /* terrestrial - to get Hz */
				d->frequency[i] *= 10;
				break;
			case 2: /* cable - to get Hz */
				d->frequency[i] *= 100;
				break;
			case 0: /* not defined */
			default:
				break;
		}
	}
}

void dvb_desc_frequency_list_print(struct dvb_v5_fe_parms *parms, const struct dvb_desc *desc)
{
	const struct dvb_desc_frequency_list *d = (const struct dvb_desc_frequency_list *) desc;
	dvb_log("|       frequency list type: %d", d->freq_type);
	int i = 0;

	for (i = 0; i < d->frequencies; i++) {
		dvb_log("|       frequency : %d", d->frequency[i]);
	}
}

