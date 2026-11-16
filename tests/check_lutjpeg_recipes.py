"""Check colour behaviour and recipe constraints independently of RAW decoding."""
import copy
import hashlib
import json
from pathlib import Path
import sys
import unittest

root = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(root / 'tools'))
from lutjpeg_recipe import transform, validate

spec = json.loads((root / 'firmware/data/lutjpeg/RECIPES.json').read_text())
centres = spec['sector_centres_degrees']


class RecipeTests(unittest.TestCase):
    def test_identity_and_neutral_matrix(self):
        r = copy.deepcopy(spec['looks'][1])
        r.update(matrix=[[1, 0, 0], [0, 1, 0], [0, 0, 1]], curve=[[0, 0], [1, 1]],
                 hue_degrees=[0]*12, saturation_scale=[1]*12, lightness_delta=[0]*12)
        for rgb in ([0, 0, 0], [1, 1, 1], [.2, .4, .8], [1, 0, 0]):
            self.assertLess(max(abs(a-b) for a, b in zip(rgb, transform(rgb, r, centres))), 1e-10)
        r['matrix'] = spec['looks'][0]['matrix']
        for x in range(101):
            self.assertLess(max(abs(v-x/100) for v in transform([x/100]*3, r, centres)), 1e-10)

    def test_monochrome_and_tone_ramp(self):
        r = spec['looks'][2]
        ramp = [transform([x/255]*3, r, centres)[0] for x in range(256)]
        self.assertTrue(all(a <= b for a, b in zip(ramp, ramp[1:])))
        self.assertAlmostEqual(ramp[0], .008)
        self.assertAlmostEqual(ramp[-1], .99)
        for rgb in ([1, 0, 0], [0, 1, 0], [0, 0, 1], [.8, .4, .2]):
            out = transform(rgb, r, centres)
            self.assertEqual(out, [out[0]]*3)

    def test_hue_wrap_and_gamut(self):
        for r in spec['looks']:
            a = transform([.8, .2, .20001], r, centres)
            b = transform([.8, .20001, .2], r, centres)
            self.assertLess(max(abs(x-y) for x, y in zip(a, b)), .001)
            for red in (0, .1, .5, 1):
                for green in (0, .1, .5, 1):
                    for blue in (0, .1, .5, 1):
                        self.assertTrue(all(0 <= v <= 1 for v in transform([red, green, blue], r, centres)))

    def test_manifest_and_validation(self):
        validate(spec)
        manifest = json.loads((root/'firmware/data/lutjpeg/TABLES.json').read_text())
        self.assertEqual(manifest['recipe_sha256'], hashlib.sha256((root/'firmware/data/lutjpeg/RECIPES.json').read_bytes()).hexdigest())
        for name, digest in manifest['sha256'].items():
            self.assertEqual(digest, hashlib.sha256((root/'firmware/data/lutjpeg'/name).read_bytes()).hexdigest())
        for field, value in [('matrix', [[1, 1, 1]]*3), ('curve', [[0, 1], [1, 0]]), ('hue_degrees', [0]*11)]:
            bad = copy.deepcopy(spec)
            bad['looks'][0][field] = value
            with self.assertRaises(AssertionError):
                validate(bad)


if __name__ == '__main__':
    unittest.main()
