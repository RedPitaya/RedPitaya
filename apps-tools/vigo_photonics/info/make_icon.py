from PIL import Image, ImageDraw, ImageFilter

LOGO = "../img/vigo_logo.png"
NAVY = (3, 5, 94, 255)
# Measured off the stock icons: 63 px of corner radius on a 256 px plate.
RADIUS_RATIO = 63.0 / 256.0  # same corner radius as the stock icons
SUPERSAMPLE = 4


def lockup():
    """The brand logo as drawn in the header: white artwork, no background."""
    logo = Image.open(LOGO).convert("RGBA")
    return logo.crop(logo.getbbox())


def render(size):
    """Draws at a multiple of the target size, so the outline comes out smooth."""
    big_size = size * SUPERSAMPLE
    radius = int(round(big_size * RADIUS_RATIO))

    plate = Image.new("RGBA", (big_size, big_size), (0, 0, 0, 0))
    ImageDraw.Draw(plate).rounded_rectangle([0, 0, big_size - 1, big_size - 1], radius=radius,
                                            fill=NAVY)

    art = lockup()
    width = int(big_size * 0.76)
    height = max(1, int(round(width * art.size[1] / art.size[0])))
    art = art.resize((width, height), Image.LANCZOS)
    plate.alpha_composite(art, ((big_size - width) // 2, (big_size - height) // 2))

    # Edge shading following the outline, as on the stock icons.
    inner = Image.new("L", (big_size, big_size), 0)
    inset = max(2, int(big_size * 0.06))
    ImageDraw.Draw(inner).rounded_rectangle(
        [inset, inset, big_size - 1 - inset, big_size - 1 - inset],
        radius=max(1, radius - inset), fill=255)
    inner = inner.filter(ImageFilter.GaussianBlur(big_size * 0.075))

    shade = Image.new("RGBA", (big_size, big_size), (0, 0, 0, 255))
    shade.putalpha(Image.eval(inner, lambda v: int((255 - v) * 0.55)))
    plate.alpha_composite(shade)

    outline = Image.new("RGBA", (big_size, big_size), (0, 0, 0, 0))
    ImageDraw.Draw(outline).rounded_rectangle(
        [0, 0, big_size - 1, big_size - 1], radius=radius, outline=(0, 0, 30, 190),
        width=max(1, int(big_size * 0.012)))
    plate.alpha_composite(outline)

    mask = Image.new("L", (big_size, big_size), 0)
    ImageDraw.Draw(mask).rounded_rectangle([0, 0, big_size - 1, big_size - 1], radius=radius,
                                           fill=255)
    plate.putalpha(Image.composite(plate.getchannel("A"),
                                   Image.new("L", (big_size, big_size), 0), mask))

    return plate.resize((size, size), Image.LANCZOS)


for size in (128, 256, 512):
    render(size).save(f"icon/{size}.png")
render(256).save("icon.png")
