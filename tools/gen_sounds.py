#!/usr/bin/env python3
"""Generate the HalfWorm sound effects (mono, 16-bit, 22050 Hz WAV) into ..\\sounds.

Plain synthesis (sine/square sweeps, filtered noise, envelopes); no samples are used,
so the result is free of any third party copyright.  Run:  python tools\\gen_sounds.py
"""
import math
import os
import random
import struct
import wave

RATE = 22050
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'sounds')
random.seed(1997)


def env(i, n, attack=0.005, curve=3.0):
    """Fast attack, exponential-ish decay."""
    t = i / RATE
    a = min(1.0, t / attack) if attack > 0 else 1.0
    return a * (1.0 - i / n) ** curve


def sweep(f0, f1, dur, shape='sine', curve=3.0, vol=0.8):
    n = int(RATE * dur)
    out, ph = [], 0.0
    for i in range(n):
        f = f0 + (f1 - f0) * (i / n)
        ph += 2 * math.pi * f / RATE
        s = math.sin(ph) if shape == 'sine' else (1.0 if math.sin(ph) >= 0 else -1.0)
        out.append(s * env(i, n, curve=curve) * vol)
    return out


def noise(dur, lp=0.3, curve=2.0, vol=0.9):
    """Low-pass filtered noise burst; lp near 0 = dull rumble, near 1 = hiss."""
    n = int(RATE * dur)
    out, y = [], 0.0
    for i in range(n):
        y += lp * (random.uniform(-1, 1) - y)
        out.append(y * env(i, n, curve=curve) * vol)
    return out


def mix(*parts):
    n = max(len(p) for p in parts)
    return [sum(p[i] for p in parts if i < len(p)) for i in range(n)]


def seq(*parts):
    out = []
    for p in parts:
        out.extend(p)
    return out


def write(name, samples):
    peak = max(1e-9, max(abs(s) for s in samples))
    scale = 0.9 * 32767 / max(peak, 0.9)
    data = b''.join(struct.pack('<h', int(max(-32767, min(32767, s * scale)))) for s in samples)
    with wave.open(os.path.join(OUT, name), 'wb') as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(RATE)
        w.writeframes(data)
    print('%-26s %5.2f s' % (name, len(samples) / RATE))


os.makedirs(OUT, exist_ok=True)

# explosions: bigger number = bigger, longer, lower
for i in range(5):
    dur = 0.25 + 0.17 * i
    lp = 0.55 - 0.08 * i
    write('explosion%d.wav' % i,
          mix(noise(dur, lp=lp, curve=2.2), sweep(140 - 18 * i, 35, dur, curve=2.0, vol=0.7)))

write('bullet_bounce_wall.wav', mix(sweep(900, 600, 0.09, 'square', curve=3, vol=0.35), noise(0.04, 0.9, vol=0.3)))
write('bullet_bounce_worm.wav', seq(sweep(500, 380, 0.06, 'sine', vol=0.7), sweep(360, 260, 0.08, 'sine', vol=0.6)))
write('bullet_bounce_tron.wav', seq(sweep(1400, 1800, 0.05, 'square', vol=0.3), sweep(1800, 1100, 0.07, 'square', vol=0.3)))
write('worm_shoot.wav', mix(sweep(1800, 300, 0.16, 'square', curve=2.5, vol=0.35), noise(0.05, 0.8, curve=3, vol=0.4)))
write('worm_died.wav', mix(sweep(420, 60, 0.7, 'square', curve=1.5, vol=0.35), noise(0.5, 0.2, curve=1.5, vol=0.5)))
write('upgraded_weaponary.wav',
      seq(*[sweep(f, f, 0.08, 'sine', curve=1.2, vol=0.6) for f in (523, 659, 784, 1047)]))
write('ate_apple.wav', seq(sweep(300, 600, 0.06, 'sine', curve=1.0, vol=0.7), sweep(600, 900, 0.08, 'sine', vol=0.6)))
