# Emacs

[TOC]

## Debugging

[Linux Debugging](linux_debugging.md) has some emacs-specific debugging tips.


## Blink Style (WebKit)

Chrome and Blink/WebKit style differ. You can use
[directory-local variables](http://www.gnu.org/software/emacs/manual/html_node/emacs/Directory-Variables.html)
to make the tab key do the right thing. E.g., in `third_party/WebKit`, add a
`.dir-locals.el` that contains

```el
((nil . ((indent-tabs-mode . nil)
         (c-basic-offset . 4)
         (fill-column . 120))))
```

This turns off tabs, sets your indent to four spaces, and makes `M-q` wrap at
120 columns (WebKit doesn't define a wrap column, but there's a soft limit
somewhere in that area for comments. Some reviewers do enforce the no wrap
limit, which Emacs can deal with gracefully; see below.)

Be sure to `echo .dir-locals.el >> .git/info/exclude` so `git clean` doesn't
delete your file.

It can be useful to set up a WebKit specific indent style. It's not too much
different so it's easy to base off of the core Google style. Somewhere after
you've loaded google.el (most likely in your .emacs file), add:

```el
(c-add-style "WebKit" '("Google"
                        (c-basic-offset . 4)
                        (c-offsets-alist . ((innamespace . 0)
                                            (access-label . -)
                                            (case-label . 0)
                                            (member-init-intro . +)
                                            (topmost-intro . 0)
                                            (arglist-cont-nonempty . +)))))
```

then you can add

```el
(c-mode . ((c-file-style . "WebKit")))
(c++-mode . ((c-file-style . "WebKit"))))
```

to the end of the .dir-locals.el file you created above. Note that this style
may not yet be complete, but it covers the most common differences.

Now that you have a WebKit specific style being applied, and assuming you have
font locking and it's default jit locking turned on, you can also get Emacs 23
to wrap long lines more intelligently by adding the following to your .emacs
file:

```el
;; For dealing with WebKit long lines and word wrapping.
(defun c-mode-adaptive-indent (beg end)
  "Set the wrap-prefix for the region between BEG and END with adaptive filling."
  (goto-char beg)
  (while
      (let ((lbp (line-beginning-position))
            (lep (line-end-position)))
        (put-text-property lbp lep 'wrap-prefix (concat (fill-context-prefix lbp lep) (make-string c-basic-offset ? )))
        (search-forward "\n" end t))))

(define-minor-mode c-adaptive-wrap-mode
  "Wrap the buffer text with adaptive filling for c-mode."
  :lighter ""
  (save-excursion
    (save-restriction
      (widen)
      (let ((buffer-undo-list t)
            (inhibit-read-only t)
            (mod (buffer-modified-p)))
        (if c-adaptive-wrap-mode
            (jit-lock-register 'c-mode-adaptive-indent)
          (jit-lock-unregister 'c-mode-adaptive-indent)
          (remove-text-properties (point-min) (point-max) '(wrap-prefix pref)))
        (restore-buffer-modified-p mod)))))

(defun c-adaptive-wrap-mode-for-webkit ()
  "Turn on visual line mode and adaptive wrapping for WebKit source files."
  (if (or (string-equal "webkit" c-indentation-style)
          (string-equal "WebKit" c-indentation-style))
      (progn
        (visual-line-mode t)
        (c-adaptive-wrap-mode t))))

(add-hook 'c-mode-common-hook 'c-adaptive-wrap-mode-for-webkit)
(add-hook 'hack-local-variables-hook 'c-adaptive-wrap-mode-for-webkit)
```

This turns on visual wrap mode for files using the WebKit c style, and sets up a
hook to dynamically set the indent on the wrapped lines. It's not quite as
intelligent as it could be (e.g., what would the wrap be if there really were a
newline there?), but it's very fast. It makes dealing with long code lines
anywhere much more tolerable (not just in WebKit).

## Syntax-error Highlighting

[Ninja](ninja_build.md) users get in-line highlighting of syntax errors using
`flymake.el` on each buffer-save:


    (load-file "src/tools/emacs/flymake-chromium.el")


## [ycmd](https://github.com/Valloric/ycmd) (YouCompleteMe) + flycheck

[emacs-ycmd](https://github.com/abingham/emacs-ycmd) in combination with
flycheck provides:

*   advanced code completion
*   syntax checking
*   navigation to declarations and definitions (using `ycmd-goto`) based on
    on-the-fly processing using clang. A quick demo video showing code
    completion and flycheck highlighting a missing semicolon syntax error:

[![video preview][img]][video]

[img]: http://img.youtube.com/vi/a0zMbm4jACk/0.jpg
[video]: http://www.youtube.com/watch?feature=player_embedded&v=a0zMbm4jACk

### Requirements

  * Your build system is set up for building with clang or wrapper+clang

### Setup

1.  Clone, update external git repositories and build.sh ycmd from
    https://github.com/Valloric/ycmd into a directory, e.g. `~/dev/ycmd`
1.  Test `ycmd` by running `~/dev/ycmd$ python ycmd/__main__.py` You should see
    `KeyError: 'hmac_secret'`
1.  Install the following packages to emacs, for example from melpa:
    *   `ycmd`
    *   `company-ycmd`
    *   `flycheck-ycmd`
1.  [More info on configuring emacs-ycmd](https://github.com/abingham/emacs-ycmd#quickstart)
    1.  Assuming your checkout of Chromium is in `~/dev/blink`, i.e. this is the
        directory in which you find the `src`folder, create a symbolic link as
        follows:

        ```shell
        cd ~/dev/blink
        ln -s src/tools/vim/chromium.ycm_extra_conf.py .ycm_extra_conf.py
        ```

    1.  Add something like the following to your `init.el`

```el
;; ycmd

;;; Googlers can replace a lot of this with (require 'google-ycmd).

(require 'ycmd)
(require 'company-ycmd)
(require 'flycheck-ycmd)

(company-ycmd-setup)
(flycheck-ycmd-setup)

;; Show completions after 0.15 seconds
(setq company-idle-delay 0.15)

;; Activate for editing C++ files
(add-hook 'c++-mode-hook 'ycmd-mode)
(add-hook 'c++-mode-hook 'company-mode)
(add-hook 'c++-mode-hook 'flycheck-mode)

;; Replace the directory information with where you downloaded ycmd to
(set-variable 'ycmd-server-command (list "python" (substitute-in-file-name "$HOME/dev/ycmd/ycmd/__main__.py")))

;; Edit according to where you have your Chromium/Blink checkout
(add-to-list 'ycmd-extra-conf-whitelist (substitute-in-file-name "$HOME/dev/blink/.ycm_extra_conf.py"))

;; Show flycheck errors in idle-mode as well
(setq ycmd-parse-conditions '(save new-line mode-enabled idle-change))

;; Makes emacs-ycmd less verbose
(setq url-show-status nil)
```

### Troubleshooting

*   If no completions show up or emacs reports errors, you can check the
    `*ycmd-server*` buffer for errors. See the next bullet point for how to
    handle "OS Error: No such file or directory"
*   Launching emacs from an OS menu might result in a different environment so
    that `ycmd` does not find ninja. In that case, you can use a package like
    [exec-path from shell](https://github.com/purcell/exec-path-from-shell) and
    add the following to your `init.el`:

```el
(require 'exec-path-from-shell)
(when (memq window-system '(mac ns x))
    (exec-path-from-shell-initialize))
```

## ff-get-other-file

There's a builtin function called `ff-get-other-file` which will get the "other
file" based on file extension. I have this bound to C-o in c-mode
(`(local-set-key "\C-o" 'ff-get-other-file)`). While "other file" is per-mode
defined, in c-like languages it means jumping between the header and the source
file. So I switch back and forth between the header and the source with C-o. If
we had separate include/ and src/ directories, this would be a pain to setup,
but this might just work out of the box for you. See the documentation for the
variable `cc-other-file-alist` for more information.

One drawback of ff-get-other-file is that it will always switch to a matching
buffer, even if the other file is in a different directory, so if you have
A.cc,A.h,A.cc(2) then ff-get-other-file will switch to A.h from A.cc(2) rather
than load A.h(2) from the appropriate directory. If you prefer something (C
specific) that always finds, try this:

```el
(defun cc-other-file()
  "Toggles source/header file"
  (interactive)
  (let ((buf (current-buffer))
        (name (file-name-sans-extension (buffer-file-name)))
	(other-extens
	 (cadr (assoc (concat "\\."
			      (file-name-extension (buffer-file-name))
			      "\\'")
		      cc-other-file-alist))))
    (dolist (e other-extens)
      (if (let ((f (concat name e)))
	    (and (file-exists-p f) (find-file f)))
	  (return)))
    )
  )
```

_Note: if you know an easy way to change the ff-get-other-file behavior, please
replace this hack with that solution! - stevenjb@chromium.org_

## Use Google's C++ style!

We have an emacs module,
[google-c-style.el](http://google-styleguide.googlecode.com/svn/trunk/google-c-style.el),
which adds c-mode formatting. Then add to your .emacs:

```el
(load "/<path/to/chromium>/src/buildtools/clang_format/script/clang-format.el")
(add-hook 'c-mode-common-hook
    (function (lambda () (local-set-key (kbd "TAB") 'clang-format-region))))
```

Now, you can use the

&lt;Tab&gt;

key to format the current line (even a long line) or region.

## Highlight long lines

One nice way to highlight long lines and other style issues:

```el
(require 'whitespace)
(setq whitespace-style '(face indentation trailing empty lines-tail))
(setq whitespace-line-column nil)
(set-face-attribute 'whitespace-line nil
                    :background "purple"
                    :foreground "white"
                    :weight 'bold)
(global-whitespace-mode 1)
```

Note: You might need to grab the latest version of
[whitespace.el](http://www.emacswiki.org/emacs-en/download/whitespace.el).

## gyp

### `gyp` style
There is a gyp mode that provides basic indentation and font-lock (syntax
highlighting) support. The mode derives from python.el (bundled with newer
emacsen).

You can find it in /src/tools/gyp/tools/emacs

See the README file there for installation instructions.

**Important**: the mode is only tested with `python.el` (bundled with newer
emacsen), not with `python-mode.el` (outdated and less maintained these days).

### deep nesting

A couple of helpers that show a summary of where you are; the first by tracing
the indentation hierarchy upwards, the second by only showing `#if`s and
`#else`s that are relevant to the current line:

```el
(defun ami-summarize-indentation-at-point ()
  "Echo a summary of how one gets from the left-most column to
  POINT in terms of indentation changes."
  (interactive)
  (save-excursion
    (let ((cur-indent most-positive-fixnum)
          (trace '()))
      (while (not (bobp))
        (let ((current-line (buffer-substring (line-beginning-position)
                                              (line-end-position))))
          (when (and (not (string-match "^\\s-*$" current-line))
                     (< (current-indentation) cur-indent))
            (setq cur-indent (current-indentation))
            (setq trace (cons current-line trace))
            (if (or (string-match "^\\s-*}" current-line)
                    (string-match "^\\s-*else " current-line)
                    (string-match "^\\s-*elif " current-line))
                (setq cur-indent (1+ cur-indent)))))
        (forward-line -1))
      (message "%s" (mapconcat 'identity trace "\n")))))

(require 'cl)
  (defun ami-summarize-preprocessor-branches-at-point ()
    "Summarize the C preprocessor branches needed to get to point."
    (interactive)
    (flet ((current-line-text ()
              (buffer-substring (line-beginning-position) (line-end-position))))
      (save-excursion
        (let ((eol (or (end-of-line) (point)))
              deactivate-mark directives-stack)
          (goto-char (point-min))
          (while (re-search-forward "^#\\(if\\|else\\|endif\\)" eol t)
            (if (or (string-prefix-p "#if" (match-string 0))
                    (string-prefix-p "#else" (match-string 0)))
                (push (current-line-text) directives-stack)
            (if (string-prefix-p "#endif" (match-string 0))
                 (while (string-prefix-p "#else" (pop directives-stack)) t))))
          (message "%s" (mapconcat 'identity (reverse directives-stack) "\n"))))))
```

## find-things-fast

erg wrote a suite of tools that do common operations from the root of your
repository, called
[Find Things Fast](https://github.com/eglaysher/find-things-fast). It contains
ido completion over `git ls-files` (or the svn find equivalent) and `grepsource`
that only git greps files with extensions we care about (or the equivalent the
`find | xargs grep` statement in non-git repos.)

## vc-mode and find-file performance

When you first open a file under git control, vc mode kicks in and does a high
level stat of your git repo. For huge repos, especially WebKit and Chromium,
this makes opening a file take literally seconds. This snippet disables VC git
for chrome directories:

```el
; Turn off VC git for chrome
(when (locate-library "vc")
(defadvice vc-registered (around nochrome-vc-registered (file))
(message (format "nochrome-vc-registered %s" file))
(if (string-match ".*chrome/src.*" file)
(progn
(message (format "Skipping VC mode for %s" % file))
(setq ad-return-value nil)
)
ad-do-it)
)
(ad-activate 'vc-registered)
)
```

## git tools

We're collecting Chrome-specific tools under `tools/emacs`. See the files there
for details.

*   `trybot.el`: import Windows trybot output into a `compilation-mode` buffer.

## ERC for IRC

See [ErcIrc](erc_irc.md).

## TODO

*   Figure out how to make `M-x compile` default to
    `cd /path/to/chrome/root; make -r chrome`.
