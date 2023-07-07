/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS 1000
#define NUM_FAN_LEVELS 3
#define NUM_FAN_STATE_LEVELS 4

#define FAN_1_LEVEL_1 DT_ALIAS(fan_1_level_1)
#define FAN_1_LEVEL_2 DT_ALIAS(fan_1_level_2)
#define FAN_1_LEVEL_3 DT_ALIAS(fan_1_level_3)

#define FAN_2_LEVEL_1 DT_ALIAS(fan_2_level_1)
#define FAN_2_LEVEL_2 DT_ALIAS(fan_2_level_2)
#define FAN_2_LEVEL_3 DT_ALIAS(fan_2_level_3)

#define SW0_NODE DT_ALIAS(sw0)
#define SW1_NODE DT_ALIAS(sw1)

static const struct gpio_dt_spec fan_1_level_1 = GPIO_DT_SPEC_GET(FAN_1_LEVEL_1, gpios);
static const struct gpio_dt_spec fan_1_level_2 = GPIO_DT_SPEC_GET(FAN_1_LEVEL_2, gpios);
static const struct gpio_dt_spec fan_1_level_3 = GPIO_DT_SPEC_GET(FAN_1_LEVEL_3, gpios);

static const struct gpio_dt_spec fan_2_level_1 = GPIO_DT_SPEC_GET(FAN_2_LEVEL_1, gpios);
static const struct gpio_dt_spec fan_2_level_2 = GPIO_DT_SPEC_GET(FAN_2_LEVEL_2, gpios);
static const struct gpio_dt_spec fan_2_level_3 = GPIO_DT_SPEC_GET(FAN_2_LEVEL_3, gpios);

static const struct gpio_dt_spec button1 = GPIO_DT_SPEC_GET(SW0_NODE, gpios);
static const struct gpio_dt_spec button2 = GPIO_DT_SPEC_GET(SW1_NODE, gpios);

struct fan
{
	const struct gpio_dt_spec *level_pins[NUM_FAN_LEVELS];
	int current_level;
};
struct button
{
	const struct gpio_dt_spec *pin;
	void (*callback)(const struct device *dev, struct gpio_callback *cb, uint32_t pins);
};

struct fan fan_1 = {
		.level_pins = {&fan_1_level_1, &fan_1_level_2, &fan_1_level_3},
		.current_level = -1};

struct fan fan_2 = {
		.level_pins = {&fan_2_level_1, &fan_2_level_2, &fan_2_level_3},
		.current_level = -1};

struct button button_1 = {
		.pin = &button1,
		.callback = NULL};

struct button button_2 = {
		.pin = &button2,
		.callback = NULL};

void button_1_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{

	fan_1.current_level = (fan_1.current_level + 1) % NUM_FAN_STATE_LEVELS;
	for (int i = 0; i < NUM_FAN_LEVELS; i++)
	{
		printk("fan level: %d \n", (i == fan_1.current_level) ? 1 : 0);
		printk("pin %d \n", fan_1.level_pins[i]->pin);
		gpio_pin_set(dev, fan_1.level_pins[i]->pin, (i == fan_1.current_level) ? 1 : 0);
	}
	printk("Fan 1 level: %d\n", fan_1.current_level + 1);
}

void button_2_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	fan_2.current_level = (fan_2.current_level + 1) % NUM_FAN_STATE_LEVELS;
	for (int i = 0; i < NUM_FAN_LEVELS; i++)
	{
		gpio_pin_set(dev, fan_2.level_pins[i]->pin, (i == fan_2.current_level) ? 1 : 0);
	}
	printk("Fan 2 level: %d\n", fan_2.current_level + 1);
}

static struct gpio_callback button_1_cb_data;
static struct gpio_callback button_2_cb_data;

void setup_button(const struct gpio_dt_spec *button_pin, void (*callback)(const struct device *dev, struct gpio_callback *cb, uint32_t pins))
{
	gpio_pin_configure_dt(button_pin, GPIO_INPUT);
	gpio_pin_interrupt_configure_dt(button_pin, GPIO_INT_EDGE_TO_ACTIVE);
	gpio_init_callback(callback == button_1_callback ? &button_1_cb_data : &button_2_cb_data, callback, BIT(button_pin->pin));
	gpio_add_callback(button_pin->port, callback == button_1_callback ? &button_1_cb_data : &button_2_cb_data);
}

void setup_fan(const struct fan *fan)
{
	for (int i = 0; i < NUM_FAN_LEVELS; i++)
	{
		gpio_pin_configure_dt(fan->level_pins[i], GPIO_OUTPUT_ACTIVE);
		gpio_pin_set(fan->level_pins[i]->port, fan->level_pins[i]->pin, 0);
	}
}

void main(void)
{

	setup_fan(&fan_1);
	setup_fan(&fan_2);

	setup_button(button_1.pin, button_1_callback);
	setup_button(button_2.pin, button_2_callback);

	while (1)
	{
		k_msleep(SLEEP_TIME_MS);
	}
}