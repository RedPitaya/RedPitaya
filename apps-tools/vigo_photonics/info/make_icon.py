from PIL import Image, ImageDraw, ImageFilter, ImageFont

FONT = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"
WHITE = (255, 255, 255, 255)
NAVY = (13, 13, 92, 255)
# Measured off the stock icons: 63 px of corner radius on a 256 px plate.
RADIUS_RATIO = 63.0 / 256.0  # same corner radius as the stock icons
SUPERSAMPLE = 4


def fitted(draw, text, target, start):
    for points in range(start, 6, -1):
        font = ImageFont.truetype(FONT, points)
        left, _, right, _ = draw.textbbox((0, 0), text, font=font)
        if right - left <= target:
            return font
    return ImageFont.truetype(FONT, 8)


def render(size):
    """Draws at a multiple of the target size, so the outline comes out smooth."""
    big_size = size * SUPERSAMPLE
    radius = int(round(big_size * RADIUS_RATIO))

    plate = Image.new("RGBA", (big_size, big_size), (0, 0, 0, 0))
    ImageDraw.Draw(plate).rounded_rectangle([0, 0, big_size - 1, big_size - 1], radius=radius,
                                            fill=NAVY)

    layer = Image.new("RGBA", (big_size, big_size), (0, 0, 0, 0))
    pen = ImageDraw.Draw(layer)
    big = fitted(pen, "VIGO", int(big_size * 0.66), int(big_size * 0.56))
    small_text = "P H O T O N I C S"
    small = fitted(pen, small_text, int(big_size * 0.72), int(big_size * 0.185))
    pen.text((big_size // 2, int(big_size * 0.42)), "VIGO", font=big, fill=WHITE, anchor="mm")
    pen.text((big_size // 2, int(big_size * 0.66)), small_text, font=small, fill=WHITE, anchor="mm")
    box = layer.getbbox()
    dx = (big_size - (box[2] - box[0])) // 2 - box[0]
    dy = (big_size - (box[3] - box[1])) // 2 - box[1]
    plate.alpha_composite(layer.transform(layer.size, Image.AFFINE, (1, 0, -dx, 0, 1, -dy)))

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
