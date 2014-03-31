# mod_hoedown

mod_hoedown is Markdown handler module for Apache HTTPD Server.

## Dependencies

* [hoextdown](https://github.com/kjdev/hoextdown.git)

## Build

```
% git clone --recursive --depth=1 https://github.com/kjdev/apache-mod-hoedown.git
% cd apache-mod-hoedown
% ./autogen.sh (or autoreconf -i)
% ./configure [OPTION]
% make
% make install
```

### Build options

Incorporation of external file.

* --enable-hoedown-url-support

  access: `http://localhost/none.md?url=https://raw.github.com/kjdev/apache-mod-hoedown/master/README.md`

apache path.

* --with-apxs=PATH
* --with-apr=PATH
* --with-apreq2=PATH

## Configration

httpd.conf:

```
LoadModule hoedown_module modules/mod_hoedown.so
<IfModule hoedown_module>
    AddHandler hoedown .md
    # <Location /markdown>
    #     SetHandler hoedown
    # </Location>
</IfModule
```

Set the "hoedown" to `SetHandler` or `AddHander`.


### Options

String:

* [HoedownDefaultPage](#hoedowndefaultpage)
* [HoedownDirectoryIndex](#hoedowndirectoryindex)
* [HoedownStylePath](#hoedownstylepath)
* [HoedownStyleDefault](#hoedownstyledefault)
* [HoedownStyleExtension](#hoedownstyleextension)
* [HoedownClassUl](#hoedownclassul)
* [HoedownClassOl](#hoedownClassol)
* [HoedownClassTask](#hoedownclasstask)
* [HoedownTocHeader](#hoedowntocheader)
* [HoedownTocFooter](#hoedowntocfooter)

Numeric:

* [HoedownTocBegin](#hoedowntocbegin)
* [HoedownTocEnd](#hoedowntocend)

On/Off:

* [HoedownRaw](#hoedownraw)
* [HoedownTocUnescape](#hoedowntocunescape)
* [HoedownExtSpaceHeaders](#hoedownextspaceheaders)
* [HoedownExtTables](#hoedownexttables)
* [HoedownExtFencedCode](#hoedownextfencedcode)
* [HoedownExtFootnotes](#hoedownextfootnotes)
* [HoedownExtAutolink](#hoedownextautolink)
* [HoedownExtStrikethrough](#hoedownextstrikethrough)
* [HoedownExtUnderline](#hoedownextunderline)
* [HoedownExtHighlight](#hoedownexthighlight)
* [HoedownExtQuote](#hoedownextquote)
* [HoedownExtSuperscript](#hoedownextsuperscript)
* [HoedownExtLaxSpacing](#hoedownextlaxspacing)
* [HoedownExtNoIntraEmphasis](#hoedownextnointraemphasis)
* [HoedownExtDisableIndentedCode](#hoedownextdisableindentedcode)
* [HoedownExtSpecialAttribute](#hoedownextspecialattribute)
* [HoedownRenderSkipHtml](#hoedownrenderskiphtml)
* [HoedownRenderSkipStyle](#hoedownrenderskipstyle)
* [HoedownRenderSkipImages](#hoedownrenderskipimages)
* [HoedownRenderSkipLinks](#hoedownrenderskipLinks)
* [HoedownRenderExpandTabs](#hoedownrenderexpandtabs)
* [HoedownRenderSafelink](#hoedownrendersafelink)
* [HoedownRenderToc](#hoedownrendertoc)
* [HoedownRenderHardWrap](#hoedownrenderhardwrap)
* [HoedownRenderUseXhtml](#hoedownrenderusexhtml)
* [HoedownRenderEscape](#hoedownrenderescape)
* [HoedownRenderUseTaskList](#hoedownrenderusetasklist)
* [HoedownRenderLineContineu](#hoedownrenderlinecontineu)

---

### Raw option

Raw print support (default: Off).

#### HoedownRaw

file: markdown.md

```
# Header
```

access: `http://localhost/markdown.md?raw`

Enable:

```
# Header
```

Disable:

```
<h1>Header</h1>
```

---

### Extension options

Markdown extension support (default: Off).

#### HoedownExtSpaceHeaders

```
# hoge
#foo
```

Enable:

```
<h1>hoge</h1>
<p>#foo</p>
```

Disable:

```
<h1>hoge</h1>
<h1>foo</h1>
```

#### HoedownExtTables

```
First Header  | Second Header
------------- | -------------
Content Cell  | Content Cell
Content Cell  | Content Cel
```

Enable:

```
<table><thead>
<tr>
<th>First Header</th>
<th>Second Header</th>
</tr>
</thead><tbody>
<tr>
<td>Content Cell</td>
<td>Content Cell</td>
</tr>
<tr>
<td>Content Cell</td>
<td>Content Cel</td>
</tr>
</tbody></table>
```

Disable:

```
<p>First Header  | Second Header
------------- | -------------
Content Cell  | Content Cell
Content Cell  | Content Cel</p>
```

#### HoedownExtFencedCode

    ```php
    echo "hello world";
    ```

Enable:

```
<pre><code class="language-php">echo &quot;hello world&quot;;
</code></pre>
```

Disable:

```
<p><code>php
echo &quot;hello world&quot;;
</code></p>
```

#### HoedownExtFootnotes

```
Footnotes[^1] have a label[^label] and a definition[^!DEF].

[^1]: This is a footnote
[^label]: A footnote on "label"
[^!DEF]: The definition of a footnote.
```

Enable:

```
<p>Footnotes<sup id="fnref1"><a href="#fn1" rel="footnote">1</a></sup> have a label<sup id="fnref2"><a href="#fn2" rel="footnote">2</a></sup> and a definition<sup id="fnref3"><a href="#fn3" rel="footnote">3</a></sup>.</p>
```

* footer

  ```
  <div class="footnotes">
  <hr>
  <ol>
  <li id="fn1">
  <p>This is a footnote&nbsp;<a href="#fnref1" rev="footnote">&#8617;</a></p>
  </li>
  <li id="fn2">
  <p>A footnote on &quot;label&quot;&nbsp;<a href="#fnref2" rev="footnote">&#8617;</a></p>
  </li>
  <li id="fn3">
  <p>The definition of a footnote.&nbsp;<a href="#fnref3" rev="footnote">&#8617;</a></p>
  </li>
  </ol>
  </div>
  ```

Disable:

```
<p>Footnotes[^1] have a label[^label] and a definition[^!DEF].</p>

<p>[^1]: This is a footnote
[^label]: A footnote on &quot;label&quot;
[^!DEF]: The definition of a footnote.</p>
```

#### HoedownExtAutolink

```
https://github.com/kjdev
```

Enable:

```
<p><a href="https://github.com/kjdev">https://github.com/kjdev</a></p>
```

Disable:

```
<p>https://github.com/kjdev</p>
```

#### HoedownExtStrikethrough

```
this is ~~good~~ bad.
```

Enable:

```
<p>this is <del>good</del> bad.</p>
```

Disable:

```
<p>this is ~~good~~ bad.</p>
```

#### HoedownExtUnderline

```
this is _good_ bad.
```

Enable:

```
<p>this is <u>good</u> bad.</p>
```

Disable:

```
<p>this is <em>good</em> bad.</p>
```

#### HoedownExtHighlight

```
this is ==good== bad.
```

Enable:

```
<p>this is <mark>good</mark> bad.</p>
```

Disable:

```
<p>this is ==good== bad.</p>
```


#### HoedownExtQuote

```
this is "good" bad.
```

Enable:

```
<p>this is <q>good</q> bad.</p>
```

Disable:

```
<p>this is &quot;good&quot; bad.</p>
```

#### HoedownExtSuperscript

```
hoge^(fuga)
hoge ^fuga
hoge ^fu^ga
```

Enable:

```
<p>hoge<sup>fuga</sup>
hoge <sup>fuga</sup>
hoge <sup>fu<sup>ga</sup></sup></p>
```

Disable:

```
<p>hoge^(fuga)
hoge ^fuga
hoge ^fu^ga</p>
```

#### HoedownExtLaxSpacing

?

#### HoedownExtNoIntraEmphasis

```
hoge_fuga_foo
```

Enable:

```
<p>hoge_fuga_foo</p>
```

Disable:

```
<p>hoge<em>fuga</em>foo</p>
```

#### HoedownExtDisableIndentedCode

```
    echo "hoge"
```

Enable:

```
<p>echo &quot;hoge&quot;</p>
```

Disable:

```
<pre><code>echo &quot;hoge&quot;
</code></pre>
```


#### HoedownExtSpecialAttribute

```
# header {.head #id1}

[kjdev](https://github.com/kjdev){#id2 .link .github}

* item1 {#item1 .item}
* item2 {.hoge #item2 .item}
```

Enable:

```
<h1 id="id1" class="head">header</h1>

<p><a href="https://github.com/kjdev" id="id2" class="link github">kjdev</a></p>

<ul>
<li id="item1" class="item">item1</li>
<li id="item2" class="hoge item">item2</li>
</ul>
```

Disable:

```
<h1>header {.head #id1}</h1>

<p><a href="https://github.com/kjdev">kjdev</a>{#id2 .link .github}</p>

<ul>
<li>item1 {#item1 .item}</li>
<li>item2 {.hoge #item2 .item}</li>
</ul>
```

---

### HTML Render options

HTML render option (default: Off).

#### HoedownRenderSkipHtml

```
hoge<span>foo</span>
```

Enable:

```
<p>hogefoo</p>
```

Disable:

```
<p>hoge<span>foo</span></p>
```

#### HoedownRenderSkipStyle

```
hoge<style>foo</style>
```

Enable:

```
<p>hogefoo</p>
```

Disable:

```
<p>hoge<style>foo</style></p>
```

#### HoedownRenderSkipImages

```
Image: <img src="test.png">
```

Enable:

```
<p>Image: </p>
```

Disable:

```
<p>Image: <img src="test.png"></p>
```

#### HoedownRenderSkipLinks

```
Link: <a href="#">here</a>
```

Enable:

```
<p>Link: here</p>
```

Disable:

```
<p>Link: <a href="#">here</a></p>
```

#### HoedownRenderExpandTabs

Does not use.

#### HoedownRenderSafelink

```
[github](https://github.com/kjdev)
[file](file:///local.file)
```

Enable:

```
<p><a href="https://github.com/kjdev">github</a>
[file](file:///local.file)</p>
```

Disable:

```
<p><a href="https://github.com/kjdev">github</a>
<a href="file:///local.file">file</a></p>
```

#### HoedownRenderToc

```
## header2-1
## header2-2
```

Enable:

```
<h2 id="header2-1">header2-1</h2>
<h2 id="header2-2">header2-2</h2>
```

* toc render:

  ```
  <ul>
  <li>
  <a href="#header2-1">header2-1</a>
  </li>
  <li>
  <a href="#header2-2">header2-2</a>
  </li>
  </ul>
  ```

Disable:

```
<h2>header2-1</h2>
<h2>header2-2</h2>
```

#### HoedownRenderHardWrap

```
hoge
foo
```

Enable:

```
<p>hoge<br>
foo</p>
```

Disable:

```
<p>hoge
foo</p>
```

#### HoedownRenderUseXhtml

```
---
```

Enable:

```
<hr/>
```

Disable:

```
<hr>
```

#### HoedownRenderEscape

```
<a href="#">here</a>
```

Enable:

```
<p>&lt;a href=&quot;#&quot;&gt;here&lt;/a&gt;</p>
```

Disable:

```
<p><a href="#">here</a></p>
```

#### HoedownRenderUseTaskList

```
* [ ] task1
* [x] task2
* [ ] task3
```

Enable:

```
<ul>
<li><input type="checkbox"> task1</li>
<li><input checked="" type="checkbox"> task2</li>
<li><input type="checkbox"> task3</li>
</ul>
```

Disable:

```
<ul>
<li>[ ] task1</li>
<li>[x] task2</li>
<li>[ ] task3</li>
</ul>
```

#### HoedownRenderLineContinue

```
hoge
foo
huga
```

Enable:

```
<p>hoge foo huga</p>
```

Disable:

```
<p>hoge
foo
huga</p>
```

### Table of Contents options

Required HoedownRenderToc option.

View the table of contents as a header and footer.

#### HoedownTocHeader

Table of contents header.

#### HoedownTocFooter

Table of contents footer.

View the table of contents as a HoedownTocEnd from HoedownTocBegin.

#### HoedownTocBegin

Table of contents begin level (default: 2).

#### HoedownTocEnd

Table of contents end level (default: 6).

```
# header1
## header2
### header3
#### header4
##### header5
###### header6
```

#### Example

default:

```
<ul>
<li>
<a href="#header2">header2</a>
<ul>
<li>
<a href="#header3">header3</a>
<ul>
<li>
<a href="#header4">header4</a>
<ul>
<li>
<a href="#header5">header5</a>
<ul>
<li>
<a href="#header6">header6</a>
</li>
</ul>
</li>
</ul>
</li>
</ul>
</li>
</ul>
</li>
</ul>
```

conf: `HoedownTocBegin 1`, `HoedownTocEnd 3`

```
<ul>
<li>
<a href="#header1">header1</a>
<ul>
<li>
<a href="#header2">header2</a>
<ul>
<li>
<a href="#header3">header3</a>
</li>
</ul>
</li>
</ul>
</li>
</ul>
```

conf: `HoedownTocHeader '<div class="toc">'`, `HoedownTocFooter '</div>'`

```
<div class="toc">
<ul>
<li>
<a href="#header2">header2</a>
<ul>
<li>
<a href="#header3">header3</a>
<ul>
<li>
<a href="#header4">header4</a>
<ul>
<li>
<a href="#header5">header5</a>
<ul>
<li>
<a href="#header6">header6</a>
</li>
</ul>
</li>
</ul>
</li>
</ul>
</li>
</ul>
</li>
</ul>
</div>
```

You can change the toc range by specifying the toc parameters.

* `http://localhot/markdown.md?toc=3`

  Same as HoedownTocBegin = 3, HoedownTocEnd = 3.

* `http://localhot/markdown.md?toc=3:5`

  Same as HoedownTocBegin = 3, HoedownTocEnd = 5.


#### HoedownRenderTocUnescape

Table of contents escapel flags.

Required HoedownRenderToc option.

```
## `Hoge`
## `Foo`
```

Enable:

```
<h2 id="-code-hoge--code-"><code>Hoge</code></h2>
<h2 id="-code-foo--code-"><code>Foo</code></h2>
```

* toc render:

  ```
  <ul>
  <li>
  <a href="#-code-hoge--code-"><code>Hoge</code></a>
  </li>
  <li>
  <a href="#-code-foo--code-"><code>Foo</code></a>
  </li>
  </ul>
  ```

Disable:

```
<h2 id="-code-hoge--code-"><code>Hoge</code></h2>
<h2 id="-code-foo--code-"><code>Foo</code></h2>
```

* toc render:

  ```
  <ul>
  <li>
  <a href="#-code-hoge--code-">&lt;code&gt;Hoge&lt;/code&gt;</a>
  </li>
  <li>
  <a href="#-code-foo--code-">&lt;code&gt;Foo&lt;/code&gt;</a>
  </li>
  </ul>
  ```

### Class options

Set the class attribute of the list.

#### HoedownClassUl

ul tag class attribute.

```
* a
```

default:

```
<ul>
<li>a</li>
</ul>
```

conf: `HoedownClassUl ul-list`

```
<ul class="ul-list">
<li>a</li>
</ul>
```

#### HoedownClassOl

ol tag class attribute.

```
1. a
```

default:

```
<ol>
<li>a</li>
</ol>
```

conf: `HoedownClassUl ol-list`

```
<ol class="ol-list">
<li>a</li>
</ol>
```

#### HoedownClassTask

Required HoedownRenderUseTaskList option.

Class attribute of the task list (ex: `* [ ]` or `* [x]`).

```
* [ ] a
```

default:

```
<ul>
<li><input type="checkbox"/> a</li>
</ul>
```

conf: `HoedownClassUl task-list`

```
<ul class="task-list">
<li><input type="checkbox"/> a</li>
</ul>
```

### Style options

Set the style layout file.

#### HoedownStylePath

Set the style layout file directory path (default: httpd Document root).

#### HoedownStyleDefault

Set the style layout file name.

#### HoedownStyleExtension

Set the style layout file extension (default: .html).

#### Example

/var/www/style/default.html:

```
<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
<title>Markdown Layout</title>
</head>
<body>
</body>
</html>
```

conf:

```
HoedownStylePath      /var/www/style
HoedownStyleDefault   default
HoedownStyleExtension .html
```

This will expand the markdown file next to the line
with the `<body>` of style.html.

#### Example multiple style

* /var/www/style/style.html
* /var/www/style/style-2.html

You can change the layout file by specifying the layout parameters.

* `http://localhot/markdown.md`

  use style: /var/www/style/default.html

* `http://localhot/markdown.md?style=style`

  use style: /var/www/style/style.html

* `http://localhot/markdown.md?style=style-2`

  use style: /var/www/style/style-2.html

## Post Markdown

You can also send a markdown Markdown content parameter. (Send to POST)

```
POST http://localhot/markdown.md
```

form:

```
<form action="none.md" method="post">
    <textarea name="markdown"></textarea>
    <input type="submit" />
</form>
```

none.md does not exists.

### Order

Load the content in order.

1. A local file
2. Markdown Parameters
3. URL parameters

Output together.

