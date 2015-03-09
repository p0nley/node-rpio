/*
 * Copyright (c) 2015 Jonathan Perkin <jonathan@perkin.org.uk>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdlib.h>
#include <unistd.h>

#include "bcm2835.h"
#include "shim.h"

/*
 * Function select.  Currently supported:
 *
 *	BCM2835_GPIO_FSEL_INPT = 0b000
 *	BCM2835_GPIO_FSEL_OUTP = 0b001
 *
 * None of the alternate functions are currently supported.
 *
 */
static int
pin_function(shim_ctx_t* ctx, shim_args_t* args)
{
	uint32_t pin, function;

	if (!shim_unpack(ctx, args,
	    SHIM_TYPE_UINT32, &pin,
	    SHIM_TYPE_UINT32, &function,
	    SHIM_TYPE_UNKNOWN)) {
		shim_throw_error(ctx, "Incorrect arguments");
		return FALSE;
	}

	switch (function) {
	case 0:
		bcm2835_gpio_fsel((uint8_t)pin, BCM2835_GPIO_FSEL_INPT);
		break;
	case 1:
		bcm2835_gpio_fsel((uint8_t)pin, BCM2835_GPIO_FSEL_OUTP);
		break;
	default:
		shim_throw_error(ctx, "Unsupported function select");
		return FALSE;
	}

	shim_args_set_rval(ctx, args, shim_integer_new(ctx, 0));

	return TRUE;
}

/*
 * Read from a pin
 */
static int
pin_read(shim_ctx_t* ctx, shim_args_t* args)
{
	uint32_t pin, rval;

	if (!shim_unpack(ctx, args,
	    SHIM_TYPE_UINT32, &pin,
	    SHIM_TYPE_UNKNOWN)) {
		shim_throw_error(ctx, "Incorrect arguments");
		return FALSE;
	}

	rval = bcm2835_gpio_lev((uint8_t)pin);

	shim_args_set_rval(ctx, args, shim_integer_new(ctx, rval));

	return TRUE;
}

/*
 * Write to a pin
 */
static int
pin_write(shim_ctx_t* ctx, shim_args_t* args)
{
	uint32_t pin, on;

	if (!shim_unpack(ctx, args,
	    SHIM_TYPE_UINT32, &pin,
	    SHIM_TYPE_UINT32, &on,
	    SHIM_TYPE_UNKNOWN)) {
		shim_throw_error(ctx, "Incorrect arguments");
		return FALSE;
	}

	if (on)
		bcm2835_gpio_set((uint8_t)pin);
	else
		bcm2835_gpio_clr((uint8_t)pin);

	shim_args_set_rval(ctx, args, shim_integer_new(ctx, 0));

	return TRUE;
}

/*
 * SPI functions.
 */
static int
spi_begin(shim_ctx_t* ctx, shim_args_t* args)
{
	bcm2835_spi_begin();

	shim_args_set_rval(ctx, args, shim_integer_new(ctx, 0));

	return TRUE;
}

static int
spi_chip_select(shim_ctx_t* ctx, shim_args_t* args)
{
	uint32_t cs;

	if (!shim_unpack(ctx, args,
	    SHIM_TYPE_UINT32, &cs,
	    SHIM_TYPE_UNKNOWN)) {
		shim_throw_error(ctx, "Incorrect arguments");
		return FALSE;
	}

	bcm2835_spi_chipSelect((uint8_t)cs);

	shim_args_set_rval(ctx, args, shim_integer_new(ctx, 0));

	return TRUE;
}

static int
spi_set_cs_polarity(shim_ctx_t* ctx, shim_args_t* args)
{
	uint32_t cs, pol;

	if (!shim_unpack(ctx, args,
	    SHIM_TYPE_UINT32, &cs,
	    SHIM_TYPE_UINT32, &pol,
	    SHIM_TYPE_UNKNOWN)) {
		shim_throw_error(ctx, "Incorrect arguments");
		return FALSE;
	}

	bcm2835_spi_setChipSelectPolarity((uint8_t)cs, (uint8_t)pol);

	shim_args_set_rval(ctx, args, shim_integer_new(ctx, 0));

	return TRUE;
}

static int
spi_set_clock_divider(shim_ctx_t* ctx, shim_args_t* args)
{
	uint32_t div;

	if (!shim_unpack(ctx, args,
	    SHIM_TYPE_UINT32, &div,
	    SHIM_TYPE_UNKNOWN)) {
		shim_throw_error(ctx, "Incorrect arguments");
		return FALSE;
	}

	bcm2835_spi_setClockDivider((uint16_t)div);

	shim_args_set_rval(ctx, args, shim_integer_new(ctx, 0));

	return TRUE;
}

static int
spi_set_data_mode(shim_ctx_t* ctx, shim_args_t* args)
{
	uint32_t mode;

	if (!shim_unpack(ctx, args,
	    SHIM_TYPE_UINT32, &mode,
	    SHIM_TYPE_UNKNOWN)) {
		shim_throw_error(ctx, "Incorrect arguments");
		return FALSE;
	}

	bcm2835_spi_setDataMode((uint8_t)mode);

	shim_args_set_rval(ctx, args, shim_integer_new(ctx, 0));

	return TRUE;
}

static int
spi_transfer(shim_ctx_t* ctx, shim_args_t* args)
{
	shim_val_t *buf;
	uint32_t len;
	char *tx, *rx;

	buf = shim_args_get(args, 0);

	if (!shim_value_is(buf, SHIM_TYPE_BUFFER)) {
		shim_throw_error(ctx, "data is not a buffer");
		return FALSE;
	}

	if (!shim_unpack_one(ctx, args, 1, SHIM_TYPE_UINT32, &len)) {
		shim_throw_error(ctx, "len is not an integer");
		return FALSE;
	}

	tx = shim_buffer_value(buf);
	rx = malloc(len);

	bcm2835_spi_transfernb(tx, rx, len);

	shim_args_set_rval(ctx, args, shim_buffer_new_copy(ctx, rx, len));

	free(rx);

	return TRUE;
}

static int
spi_end(shim_ctx_t* ctx, shim_args_t* args)
{
	bcm2835_spi_end();

	shim_args_set_rval(ctx, args, shim_integer_new(ctx, 0));

	return TRUE;
}


/*
 * Initialize the bcm2835 interface and check we have permission to access it.
 */
static int
bcm_init(shim_ctx_t* ctx, shim_args_t* args)
{
	if (geteuid() != 0) {
		shim_throw_error(ctx, "You must be root to access GPIO");
		return FALSE;
	}

	if (!bcm2835_init()) {
		shim_throw_error(ctx, "Could not initialize GPIO");
		return FALSE;
	}

	return TRUE;
}

static int
bcm_close(shim_ctx_t* ctx, shim_args_t* args)
{
	bcm2835_close();

	return TRUE;
}

int
setup(shim_ctx_t* ctx, shim_val_t* exports, shim_val_t* module)
{
	shim_fspec_t funcs[] = {
		SHIM_FS(bcm_init),
		SHIM_FS(pin_function),
		SHIM_FS(pin_read),
		SHIM_FS(pin_write),
		SHIM_FS(spi_begin),
		SHIM_FS(spi_chip_select),
		SHIM_FS(spi_set_cs_polarity),
		SHIM_FS(spi_set_clock_divider),
		SHIM_FS(spi_set_data_mode),
		SHIM_FS(spi_transfer),
		SHIM_FS(spi_end),
		SHIM_FS(bcm_close),
		SHIM_FS_END
	};

	shim_obj_set_funcs(ctx, exports, funcs);

	return TRUE;
}

SHIM_MODULE(rpio, setup)
