# NewBase

Documentation about newbase (aka Linux/Bgfx/Crossplatform port of danmakufu ph3)

Detailed documentation of what internally changes for TouhouDanmakufu (DhnCommon, DhnExecutor, DhnViewer) projects can we viewered at [Developer conversion guide](https://github.com/arves100/danmakufu/blob/newbase/newbase/Developer%20conversion%20guide.MD)

Detailed documentation of API changes for the user (aka who creates games with this program) can be viewed at [User conversion guide](https://github.com/arves100/danmakufu/blob/newbase/newbase/User%20conversion%20guide.MD)

## Expected things
- Full BGFX support, ditch Direct3D9 ENTIRELY
- Modern C++ were applicable, no more gstd::shared_ptr clones
- Keep the Dx library just a render library, no reason to hold window initialization there
- Avoid shared pointers were possible
- Try to keep the code similar to the original one (in terms of API)
- No more widechars, see notes for more informations
- Avoid using Native API as much as possible, in case it's not possible avoid them create an API that doesn't depend on them (like avoid using Windows typenames for return types) so it would be easier to convert them.

## Wide string notes
As you probably know, 99% of the operative systems out there uses UTF-8 strings, something
that Windows can't really follow (they started now with Windows10 1909 I think), which is why
MS inctroduced to us the Unicode API, which is actually UTF-16 (It was UCS-2 in pre-NT era).
While for Dhn, I had two main ideas, I could store everything in widechar (2byte per character) and let all
the native linux apis converted to utf8, or touch all the code and store everything in multibyte/char (1byte per character).

Under Linux, it only supports UTF-8, keeping Widechar would cause I regression everytime a native api is used, as you would
have to convert it (for example with libicu) to a multibyte utf8 string before using it.

SDL2 and BGFX does not support Widechars, which means that using Widechars would cause regressions even on Windows.

Windows internally converts all multibyte texts to widechar (the NT kernel internally uses widechars, called UNICODE_STRING)
you can even see that in Wine and [ReactOS](https://github.com/reactos/reactos/blob/3fa57b8ff7fcee47b8e2ed869aecaf4515603f3f/win32ss/user/user32/windows/font.c#L333).

If we use multibytes, we won't have any regression by using linux native api or library api that we depend on, and
we can internally convert multibyte to widechar in Windows and cause 0 regressions (it might even be faster than
the default conversion).

The only place where Widechar may cause a regression is by working on Windows platforms that are not NT based (Win3x, Win9x),
as Win9x requires an unicode dll to work (unicows.dll, Iwin9x doesn't really support unicode), but those systems
died long ago so there's no reason to worry about it.

Scripts: for backward compatibility with the currently existing scripts, Dhn will load by default, scripts encoded with
Shift-JIS, it will internally convert it to UTF8 and then paste it to the parser. (NOTE: the parser might be utf-16/widechar).

If the parser is not UTF-16, it would read UTF-16 texts (with BOM) and convert it to UTF-8, or leave the loading as normal.

For UTF-8 if the parser is UTF-16, it will convert UTF-8 WITH BOM texts into UTF-16, or perform no conversion.

The parser should also be able to load UTF-8 with BOM (as a way to identify if the file is UTF-8 or UTF-16).

NOTE: Dhn should be able to support both UTF-16 LE and UTF-16 BE.

The parser would mostly receive a rewrite anyway so there's free zone into that, it all depends on how much the libraries
that will compose the new parser, lexters and interpreter would like to support, I would prefer going UTF-16 rather than UTF-8 for this.
