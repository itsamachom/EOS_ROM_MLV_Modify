"""Independent offline recipe math; GPL-2.0-or-later. No Caduceus code/data copied.

The input is the M-Film model's display RGB, not sensor-linear RGB. Matrix
mixing uses sRGB-decoded values; the existing developer still owns RAW white
balance, camera colour conversion, and spatial grain/halation. These stages
are baked into the 33^3 table, with no extra camera per-pixel stages.
"""
import colorsys
import math


def clamp(x):
    return min(1.0, max(0.0, x))


def decode(x):
    return x / 12.92 if x <= 0.04045 else ((x + 0.055) / 1.055) ** 2.4


def encode(x):
    x = clamp(x)
    return 12.92 * x if x <= 0.0031308 else 1.055 * x ** (1 / 2.4) - 0.055


def curve_value(x, points):
    for (a, b), (c, d) in zip(points, points[1:]):
        if x <= c:
            return b + (d - b) * (x - a) / (c - a)
    return points[-1][1]


def validate(spec):
    centres = spec['sector_centres_degrees']
    assert len(centres) == 12 and all(0 <= x < 360 for x in centres)
    looks = spec['looks']
    assert looks and len({r['index'] for r in looks}) == len(looks)
    assert len({r['key'] for r in looks}) == len(looks)
    for r in looks:
        assert isinstance(r['index'], int) and 0 <= r['index'] < 100
        assert len(r['matrix']) == 3
        for row in r['matrix']:
            assert len(row) == 3 and abs(sum(row) - 1) < 1e-9
            assert all(math.isfinite(v) and -1 <= v <= 2 for v in row)
        points = r['curve']
        assert len(points) >= 2 and points[0][0] == 0 and points[-1][0] == 1
        assert all(len(p) == 2 and all(math.isfinite(v) and 0 <= v <= 1 for v in p) for p in points)
        assert all(a[0] < b[0] and a[1] <= b[1] for a, b in zip(points, points[1:]))
        for key, lo, hi in [('hue_degrees', -30, 30), ('saturation_scale', 0, 2), ('lightness_delta', -.1, .1)]:
            assert len(r[key]) == 12 and all(math.isfinite(v) and lo <= v <= hi for v in r[key])
        for key in ('shadow_rgb', 'highlight_rgb'):
            assert len(r[key]) == 3 and all(math.isfinite(v) and -.1 <= v <= .1 for v in r[key])
        assert isinstance(r['monochrome'], bool)


def transform(rgb, recipe, centres):
    lin = [decode(v) for v in rgb]
    mixed = [encode(sum(a * b for a, b in zip(row, lin))) for row in recipe['matrix']]
    y = sum(a * b for a, b in zip(mixed, (.2126, .7152, .0722)))
    # Add a common luma delta: neutral ramps stay neutral without division at black.
    delta = curve_value(y, recipe['curve']) - y
    toned = [clamp(v + delta) for v in mixed]
    if recipe['monochrome']:
        value = sum(a * b for a, b in zip(toned, (.2126, .7152, .0722)))
        return [value] * 3
    hue, sat, value = colorsys.rgb_to_hsv(*toned)
    weights = [math.exp(-.5 * (((hue * 360 - c + 180) % 360 - 180) / 25) ** 2) for c in centres]
    total = sum(weights)
    weights = [w / total for w in weights]
    # Suppress hue/lightness intervention near neutral; avoid coloured grey noise.
    gate = min(1.0, sat / .2)
    hue += gate * sum(w * v for w, v in zip(weights, recipe['hue_degrees'])) / 360
    sat = clamp(sat * sum(w * v for w, v in zip(weights, recipe['saturation_scale'])))
    value = clamp(value + gate * sum(w * v for w, v in zip(weights, recipe['lightness_delta'])))
    out = colorsys.hsv_to_rgb(hue % 1, sat, value)
    # Modest split colour; zero at black/white so endpoints are not tinted/clipped.
    sh = (1 - y) ** 2 * 4 * y * (1 - y)
    hl = y ** 2 * 4 * y * (1 - y)
    return [clamp(v + sh * s + hl * h) for v, s, h in zip(out, recipe['shadow_rgb'], recipe['highlight_rgb'])]


def apply_table(data, recipe, centres):
    assert len(data) == 8 + 33 ** 3 * 3 and data[:4] == b'FOL1'
    out = bytearray(data[:8])
    for i in range(8, len(data), 3):
        out.extend(int(clamp(v) * 255 + .5) for v in transform([x / 255 for x in data[i:i+3]], recipe, centres))
    return bytes(out)
