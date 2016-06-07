/*
 * Lantiq PHY drivers
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * Copyright 2013 Instrumentation Technologies d.d.
 * author Tomaž Beltram
 *
 */

#include <phy.h>

/* internal address space */
#define MMD_DEVAD_INT 0x1f
#define MMD_ACTYPE_DATA 0x4000

/* LED configuration registers */
#define INT_LED1H 0x1e4
#define INT_LED1L 0x1e5
#define INT_LED2H 0x1e6
#define INT_LED2L 0x1e7

/* LED function bits */
#define INT_LED_TX 0x01
#define INT_LED_RX 0x02
#define INT_LED_LINK10 0x1
#define INT_LED_LINK100 0x2
#define INT_LED_LINK1000 0x4

void phy_mmd_write(struct phy_device *phydev, int devad, int addr, int val)
{
	phy_write(phydev, MDIO_DEVAD_NONE, MII_MMD_CTRL, devad);
	phy_write(phydev, MDIO_DEVAD_NONE, MII_MMD_DATA, addr);
	phy_write(phydev, MDIO_DEVAD_NONE, MII_MMD_CTRL, devad | MMD_ACTYPE_DATA);
	phy_write(phydev, MDIO_DEVAD_NONE, MII_MMD_DATA, val);
}

static int phy11g_config(struct phy_device *phydev)
{
	phy_mmd_write(phydev, MMD_DEVAD_INT, INT_LED1H, 0);
	phy_mmd_write(phydev, MMD_DEVAD_INT, INT_LED1L,
		INT_LED_RX | INT_LED_TX);
	phy_mmd_write(phydev, MMD_DEVAD_INT, INT_LED2H,
		INT_LED_LINK1000 << 4 | INT_LED_LINK100);
	phy_mmd_write(phydev, MMD_DEVAD_INT, INT_LED2L,
		INT_LED_LINK10 << 4);

	return 0;
}

static struct phy_driver PHY11G_driver =  {
	.name = "PHY11G",
	.uid = 0xd565a401,
	.mask = 0xffffffff,
	.features = PHY_GBIT_FEATURES,
	.config = phy11g_config,
	.startup = genphy_startup,
	.shutdown = genphy_shutdown,
};

int phy_lantiq_init(void)
{
	phy_register(&PHY11G_driver);

	return 0;
}
