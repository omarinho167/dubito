using System;
using UnityEngine;

namespace Dubito.Presentation
{
    public static class SeatPalette
    {
        static readonly Color32[] Colors =
        {
            new Color32(0x3B, 0x82, 0xF6, 255), new Color32(0xE5, 0x48, 0x4D, 255),
            new Color32(0x30, 0xA4, 0x6C, 255), new Color32(0xF5, 0xA5, 0x24, 255),
            new Color32(0x8E, 0x4E, 0xC6, 255), new Color32(0x12, 0xA5, 0x94, 255)
        };

        public static Color Get(int seat)
        {
            if (seat < 0 || seat >= Colors.Length) throw new ArgumentOutOfRangeException(nameof(seat));
            return Colors[seat];
        }
    }
}
