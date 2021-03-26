// SPDX-License-Identifier: GPL-2.0+

#define DEBUG

#include <linux/clk.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pwm.h>

struct clk_pwm_chip {
	struct pwm_chip chip;
	struct clk *clk;
};

#define to_clk_pwm_chip(_chip) container_of(_chip, struct clk_pwm_chip, chip)

static int clk_pwm_apply(struct pwm_chip *pwm_chip, struct pwm_device *pwm,
			 const struct pwm_state *state)
{
	struct clk_pwm_chip *chip = to_clk_pwm_chip(pwm_chip);
	int ret;

	return 0;
}

static const struct pwm_ops clk_pwm_ops = {
	.apply = clk_pwm_apply,
	.owner = THIS_MODULE,
};

static int clk_pwm_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct clk_pwm_chip *chip;
	int ret;

	chip = devm_kzalloc(&pdev->dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	/* TODO: Get the clock exclusively here maybe */
	chip->clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(chip->clk)) {
		dev_err(&pdev->dev, "Failed to get clock: %ld\n", PTR_ERR(chip->clk));
		return PTR_ERR(chip->clk);
	}

	chip->chip.dev = &pdev->dev;
	chip->chip.ops = &clk_pwm_ops;
	chip->chip.of_xlate = of_pwm_xlate_with_flags;
	chip->chip.of_pwm_n_cells = 3;
	chip->chip.base = -1;

	dev_warn(&pdev->dev, "Wide pulses go brrrrrr!");

	ret = clk_prepare_enable(chip->clk);
	dev_warn(&pdev->dev, "xxx: %d\n", ret);

	//ret = clk_set_rate(chip->clk, 500000);
	//dev_warn(&pdev->dev, "xxx: %d to %ld\n", ret, clk_round_rate(chip->clk, 500000));

	ret = clk_set_duty_cycle(chip->clk, 25, 100);
	dev_warn(&pdev->dev, "xxx: %d\n", ret);

	dev_warn(&pdev->dev, "xxx: %d to %ld\n", ret, clk_round_rate(chip->clk, 500000));

	ret = pwmchip_add(&chip->chip);
	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to add pwm chip: %d\n", ret);
		//return ret;
	}

	platform_set_drvdata(pdev, chip);

	return 0;
}

static int clk_pwm_remove(struct platform_device *pdev)
{
	struct clk_pwm_chip *chip = platform_get_drvdata(pdev);

	return pwmchip_remove(&chip->chip);
}

static const struct of_device_id clk_pwm_dt_ids[] = {
	{ .compatible = "clk-pwm", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, clk_pwm_dt_ids);

static struct platform_driver clk_pwm_driver = {
	.driver = {
		.name = "clk-pwm",
		.of_match_table = clk_pwm_dt_ids,
	},
	.probe = clk_pwm_probe,
	.remove = clk_pwm_remove,
};
module_platform_driver(clk_pwm_driver);

MODULE_ALIAS("platform:clk-pwm");
MODULE_AUTHOR("Nikita Travkin <nikitos.tr@gmail.com>");
MODULE_DESCRIPTION("Clock based PWM driver");
MODULE_LICENSE("GPL v2");
