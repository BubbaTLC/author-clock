
🟡 Medium Impact

3. line_height in font_t bmfont_height() returns only the ascender (baseline). For proper multi-line word-wrap, you need ascender + descender + leading. Adding a line_height field to the struct
would remove any hardcoded guesses in the renderer.

5. Hinting control (--hinting [auto|none|native]) E-paper pixels are physically large. Disabling or switching hinting can produce noticeably crisper letterforms — especially for condensed faces.

--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

🟢 Lower Impact / Polish

6. Binary search for glyph lookup — The renderer comment already flags this. Glyphs are emitted in ASCII order so a binary search is trivial and would speed up every character draw.

7. Kerning accuracy after embolden — The (embolden + 31) // 32 advance correction is an approximation; reading face.glyph.linearHoriAdvance post-embolden would be exact.


--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

Want me to implement any of these? The literary character support (#1) is probably the most immediately visible improvement since it fixes broken rendering of real book quotes right now.