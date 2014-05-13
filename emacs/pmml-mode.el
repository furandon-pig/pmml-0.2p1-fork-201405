;;
;; PMML editing mode for Emacs    Release 0.2
;;
;;  Copyright (C) 1997,1998   Satoshi Nishimura
;;
;;  This program is free software; you can redistribute it and/or modify
;;  it under the terms of the GNU General Public License as published by
;;  the Free Software Foundation; either version 2 of the License, or
;;  (at your option) any later version.
;;
;;  This program is distributed in the hope that it will be useful,
;;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;  GNU General Public License for more details.
;;
;;  You should have received a copy of the GNU General Public License
;;  along with this program; if not, write to the Free Software
;;  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
;;

(require 'compile)
(provide 'pmml-mode)

(setq auto-mode-alist (append '(("\\.pml$" . pmml-mode)) auto-mode-alist))

(defvar pmml-note-name ["C" "C#" "D" "Eb" "E" "F" "F#" "G" "Ab" "A" "Bb" "B"]
  "Vector of note names used in step recording.") 
(defvar pmml-show-velocity nil 
  "*If t then velocity is appended to each note during step recording.")
(defvar pmml-chord-time-tolerance 5 
  "In step recording, notes of which note-on time difference is 
less than (1/100 * this value) seconds are considered as a chord.")
(defvar pmml-use-server t
  "If t, pieceserver is used.  If nil, pmml-player-command is used for
playing songs: in this case, recording is not supported.")
(defvar pmml-temp-directory "/tmp/"
  "Directory where temporary files are created.")
;(defvar pmml-player-command '("timidity" "-s16000")
; "Shell command for starting the standard MIDI file player.")
(defvar pmml-player-command '("playmidi" "-e")
  "Shell command for starting the standard MIDI file player.")
(defvar pmml-compiler-command "pmml"
  "Command path for the PMML compiler.")
(defvar pmml-server-command '("pieceserver")
  "Shell command for starting the server.")
(defvar pmml-play-with-metronome nil 
  "*If t, metronome clicks are generated while playing.")
(defvar pmml-record-with-metronome t 
  "*If t, metronome clicks are generated while recording.")
(defvar pmml-metronome-param '(1 10 75 63 75 127)
  "*A 6-element list of integers describing the style of metronome clicks. 
The 1st and 2nt elements are respectively device ID and MIDI channel 
to which metronome clicks are outputted.  The 3rd and 4th elements are
the note number and velocity of normal clicks.  The 5th and 6th elements are
the note number and velocity of accented clicks which are generated at the
beginning of each measure.") 
(defvar pmml-record-preroll 2
  "*Number of measures for playing before entering the recording point.") 
(defvar pmml-inqure-record-param t
  "*If t, some recording parameters are asked to users before recording.")
(defvar pmml-play-rtempo "1.0"
  "*Tempo maginication during playing.  This must be a string describing 
a decimal (In version 19 emacs, a floating-point number is also acceptable).") 
(defvar pmml-record-rtempo "1.0"
  "*Tempo maginication during recording.  This must be a string describing 
a decimal (In version 19 emacs, a floating-point number is also acceptable).") 

(defvar pmml-indent-width 2
  "*Indent width per brace level.")
(defvar pmml-tab-always-indent t
  "*Non-nil means TAB in PMML mode should always reindent the current line,
regardless of where in the line point is when the TAB command is used.")

(defvar pmml-mode-map (make-sparse-keymap) 
  "Keymap for PMML mode.")
(defvar pmml-track-mode-map (make-sparse-keymap) 
  "Keymap for PMML track summary mode.")

(defvar pmml-server-process nil)
(defvar pmml-status 'stopped)
(defvar pmml-temp-file nil
  "File name (w/o extension) of temporary file.")
(defvar pmml-compiling-buffer nil
  "Set to the buffer for PMML code while it is being compiled.")
(defvar pmml-compiling-file nil
  "Compiling file name.")
(defvar pmml-compiling-mode nil)
(defvar pmml-track-buffer nil)
(defvar pmml-track-associated-buffer nil)
(defvar pmml-play-tracks nil
  "A vector whose size is the maximum track number and of which each 
element is t or nil indicating the track is to be played or not.
If nil, all the tracks are played.  Buffer local.")
(defvar pmml-note-list nil)
(defvar pmml-record-marker nil)
(defvar pmml-timeout 2)

(defvar pmml-emacs-version 
  (progn
    (string-match "\\([0-9]+\\)" emacs-version)
    (string-to-int (substring emacs-version 
			      (match-beginning 1) (match-end 1)))))

(define-key pmml-mode-map "\C-c\C-s" 'pmml-step-record)
(define-key pmml-mode-map "\C-c\C-f" 'pmml-play-file)
(define-key pmml-mode-map "\C-c\C-q" 'pmml-stop)
(define-key pmml-mode-map "\C-cy"    'pmml-play-region-solo)
(define-key pmml-mode-map "\C-c\C-y" 'pmml-play-region-all)
(define-key pmml-mode-map "\C-cp"    'pmml-play-from-here-solo)
(define-key pmml-mode-map "\C-c\C-p" 'pmml-play-from-here-all)
(define-key pmml-mode-map "\C-c\C-r" 'pmml-record)
(define-key pmml-mode-map "\C-c\C-c" 'pmml-record-finish)
(define-key pmml-mode-map "\C-c\C-t" 'pmml-show-track-summary)
(define-key pmml-mode-map "\t" 'pmml-indent-command)
(define-key pmml-mode-map "{" 'electric-pmml-paren)
(define-key pmml-mode-map "}" 'electric-pmml-paren)
(define-key pmml-mode-map "[" 'electric-pmml-paren)
(define-key pmml-mode-map "]" 'electric-pmml-paren)
(define-key pmml-mode-map ")" 'electric-pmml-paren)
(define-key pmml-mode-map "\C-m" 'newline-and-indent)

(if (>= pmml-emacs-version 19)
  (condition-case ()
    (progn
      (define-key pmml-mode-map [menu-bar] (make-sparse-keymap))
      (define-key pmml-mode-map [menu-bar pmml]
	(cons "PMML" (make-sparse-keymap "PMML")))
      
      (define-key pmml-mode-map [menu-bar pmml pmml-step-record]
	'("Step Record" . pmml-step-record))
      (define-key pmml-mode-map [menu-bar pmml pmml-show-track-summary]
	'("Show Track Summary" . pmml-show-track-summary))
      (define-key pmml-mode-map [menu-bar pmml pmml-record]
	'("Record from Here" . pmml-record))
      (define-key pmml-mode-map [menu-bar pmml pmml-play-region-all]
	'("Play Region (All)" . pmml-play-region-all))
      (define-key pmml-mode-map [menu-bar pmml pmml-play-region-solo]
	'("Play Region (Solo)" . pmml-play-region-solo))
      (define-key pmml-mode-map [menu-bar pmml pmml-play-from-here-all]
	'("Play from Here (All)" . pmml-play-from-here-all))
      (define-key pmml-mode-map [menu-bar pmml pmml-play-from-here-solo]
	'("Play from Here (Solo)" . pmml-play-from-here-solo))
      (define-key pmml-mode-map [menu-bar pmml pmml-play-file]
	'("Play from the Beginning" . pmml-play-file))
      (define-key pmml-mode-map [menu-bar pmml pmml-stop]
	'("Stop" . pmml-stop)))
    (error nil)))

(defconst pmml-mode-syntax-table nil
  "Syntax table used in PMML-mode buffers.")
(if pmml-mode-syntax-table
    nil
  (setq pmml-mode-syntax-table (make-syntax-table))
  (modify-syntax-entry ?$ "_" pmml-mode-syntax-table)
  (modify-syntax-entry ?% "." pmml-mode-syntax-table)
  (modify-syntax-entry ?& "." pmml-mode-syntax-table)
  (modify-syntax-entry ?\' "\"" pmml-mode-syntax-table)
  (modify-syntax-entry ?+ "." pmml-mode-syntax-table)
  (modify-syntax-entry ?- "." pmml-mode-syntax-table)
  (modify-syntax-entry ?< "." pmml-mode-syntax-table)
  (modify-syntax-entry ?= "." pmml-mode-syntax-table)
  (modify-syntax-entry ?> "." pmml-mode-syntax-table)
  (modify-syntax-entry ?\\ "\\" pmml-mode-syntax-table)
  (modify-syntax-entry ?| "." pmml-mode-syntax-table)
  (if (< pmml-emacs-version 19)
      (progn 
	;; give up handling '//'-style comments
	(modify-syntax-entry ?/ ". 14" pmml-mode-syntax-table)
	(modify-syntax-entry ?* ". 23" pmml-mode-syntax-table))
    ;; we still have a problem on nested '/* .. */'-style comments
    ;; but we don't have any solutions. 
    (modify-syntax-entry ?/ ". 124b" pmml-mode-syntax-table)
    (modify-syntax-entry ?* ". 23" pmml-mode-syntax-table)
    (modify-syntax-entry ?\n "> b" pmml-mode-syntax-table)))

(defconst pmml-alt-syntax-table nil
  "Alternative syntax table used by PMML mode for searching a single quote.")
(if pmml-alt-syntax-table
    nil
  (setq pmml-alt-syntax-table (make-syntax-table pmml-mode-syntax-table))
  (modify-syntax-entry ?\) "." pmml-alt-syntax-table)
  (modify-syntax-entry ?\( "." pmml-alt-syntax-table)
  (modify-syntax-entry ?\} "." pmml-alt-syntax-table)
  (modify-syntax-entry ?\{ "." pmml-alt-syntax-table)
  (modify-syntax-entry ?\] "." pmml-alt-syntax-table)
  (modify-syntax-entry ?\[ "." pmml-alt-syntax-table)
  (modify-syntax-entry ?\' "(\177" pmml-alt-syntax-table))

;; a hack on compilation-sentinel function in compile.el 
;; (for version 18 only)
(if (< pmml-emacs-version 19)
    (if (not (fboundp 'compilation-sentinel-org))
	(progn
	  (fset 'compilation-sentinel-org 
		(symbol-function 'compilation-sentinel))
	  (defvar compilation-finish-function nil)
	  (defun compilation-sentinel (proc msg)
	    (if compilation-finish-function
		(funcall compilation-finish-function 
			 (process-buffer proc) msg))
	    (compilation-sentinel-org proc msg)))))


(defun pmml-mode ()
  "Major mode for editing PMML code.
\\{pmml-mode-map}"
  (interactive)
  (kill-all-local-variables)
  (use-local-map pmml-mode-map)
  (setq major-mode 'pmml-mode)
  (setq mode-name "PMML")
  (set-syntax-table pmml-mode-syntax-table)
  (set (make-local-variable 'indent-line-function) 'pmml-indent-line)
  (set (make-local-variable 'mybuffer) (current-buffer))
  (if (null pmml-temp-file)
      (setq pmml-temp-file (concat (expand-file-name 
				    (make-temp-name "pm")
				    pmml-temp-directory))))
  (make-local-variable 'pmml-play-tracks)
  ;; status name
  (set (make-local-variable 'mode-line-process) 'pmml-status)
  (set (make-local-variable 'stopped) ": Stopped")
  (set (make-local-variable 'stopping) ": Stopping")
  (set (make-local-variable 'steprec) ": Step Rec")
  (set (make-local-variable 'playing) ": Playing")
  (set (make-local-variable 'recording) ": Recording")
  (if pmml-use-server 
      (pmml-start-server))
  (run-hooks 'pmml-mode-hook))

(defun pmml-cleanup ()
  "Remove temporary files."
  (condition-case ()
      (delete-file (concat pmml-temp-file ".pml"))
    (error nil))
  (condition-case ()
      (delete-file (concat pmml-temp-file ".trk"))
    (error nil))
  (condition-case ()
      (delete-file (get-midi-fname pmml-temp-file))
    (error nil)))

(condition-case ()
    (add-hook 'kill-emacs-hook 'pmml-cleanup)
  (error nil))

(defun pmml-step-record (action)
  "Start step-recording."
  (if (not pmml-use-server)
      (error "Step recording is not supported."))
  (interactive "p")
  (pmml-really-stop)
  (setq pmml-note-list nil)
  (process-send-string pmml-server-process "I\n")
  (setq pmml-status 'steprec)
  (pmml-update-mode-line))

(defun pmml-play-file ()
  "Play the song from the beginning.
It plays all the tracks marked as `play'."
  (interactive)
  (pmml-really-stop)
  (pmml-compile (pmml-track-string pmml-play-tracks)
		(buffer-file-name (current-buffer)) 'play))

(defun pmml-stop ()
  "Stop playing/step-recording/recording."
  (interactive)
  (cond (pmml-use-server
	 (process-send-string pmml-server-process "S\n")
	 (setq pmml-status 'stopped))
	(t
	 (if (eq (process-status "pmmlplayer") 'run)
	     (interrupt-process "pmmlplayer"))
	 (setq pmml-status 'stopping)))
  (pmml-update-mode-line))

(defun pmml-really-stop ()
  (if pmml-use-server
      (pmml-stop)
    (if (and (not (eq pmml-status 'stopping))
	     (eq (process-status "pmmlplayer") 'run))
	(let ((cnt 0))
	  (message "Killing player process...")
	  (while (and (eq (process-status "pmmlplayer") 'run) 
		      (< cnt pmml-timeout))
	    (interrupt-process "pmmlplayer")
	    (sit-for 1)
	    (setq cnt (1+ cnt))
	    (discard-input)) ; to make sure that sentinal has been called
	  (if (eq cnt pmml-timeout)
	      (progn
		(message "Timeout")
		(pmml-cleanup)
		(setq pmml-status 'stopped)
		(pmml-update-mode-line)))))
    (if (eq pmml-status 'stopping)
	(let ((cnt 0))
	  (while (and (not (eq pmml-status 'stopped)) (< cnt pmml-timeout))
	    (sit-for 1)
	    (setq cnt (1+ cnt))
	    (discard-input))  ; to make sure that sentinal has been called
	  (if (eq cnt pmml-timeout)
	      (progn
		(message "Timeout")
		(pmml-cleanup)
		(setq pmml-status 'stopped)
		(pmml-update-mode-line))
	    (message ""))))))

(defun pmml-compile (cmdopts filename mode)
  (setq pmml-compiling-buffer (current-buffer))
  (setq pmml-compiling-file filename)
  (setq pmml-compiling-mode mode)
  (setq compilation-finish-function 'pmml-compilation-finish)
  (unwind-protect
      (let ((trk-file (concat pmml-temp-file ".trk")))
	(compile (concat pmml-compiler-command " "
			 cmdopts
			 " -F trkinfo " 
			 filename 
			 " >" trk-file)))
    (condition-case ()
	(delete-file trk-file)
      (error nil))))

(defun pmml-compilation-finish (buf string)
  (setq compilation-finish-function nil)
  (if (not (string-match "finished" string))
      (pmml-cleanup)
    (catch 'pmml-t1
      (let ((obuffer (current-buffer))
	    (play-success nil))
	(unwind-protect
	    (progn
	      (pmml-read-track-file (concat pmml-temp-file ".trk")
				    pmml-compiling-buffer)
	      ;;
	      (if (memq pmml-compiling-mode '(play record))
		  (let ((mid-file (get-midi-fname pmml-compiling-file))
			pr)
		    (set-buffer pmml-compiling-buffer)
		    (cond (pmml-use-server
			   (if (eq pmml-compiling-mode 'record)
			       (if (y-or-n-p "Ready for recording ")
				   (message "Type C-c C-c to finish recording or C-c C-s to cancel.")
				 (message "Recording not confirmed.")
				 (ding)
				 (throw 'pmml-t1 nil)))
			   (process-send-string pmml-server-process
			    (format "Y %s\n"
				    (mapconcat 'int-to-string 
					       pmml-metronome-param " ")))
			   (process-send-string pmml-server-process
			    (format "X %d\n"
				    (if (eq pmml-compiling-mode 'play)
					(if pmml-play-with-metronome 1 0)
				      (if pmml-record-with-metronome 1 0))))
			   (process-send-string pmml-server-process
			    (format "M %s\n" 
				    (let ((mag 
					   (if (eq pmml-compiling-mode 'play)
					       pmml-play-rtempo 
					     pmml-record-rtempo)))
				      (if (numberp mag) 
					  (int-to-string mag)
					mag))))
			   (process-send-string pmml-server-process
						(format "L %s\n" mid-file))
			   (process-send-string 
			    pmml-server-process 
			    (cdr (assoc pmml-compiling-mode 
					'((play . "P\n") (record . "R\n"))))))
			  (t
			   (setq pr (eval (append (list 'start-process 
							"pmmlplayer" 
							pmml-compiling-buffer) 
						  pmml-player-command
						  (list mid-file))))
			   (set-process-filter pr 'pmml-player-filter)
			   (set-process-sentinel pr 'pmml-player-sentinel)))
		    (setq pmml-status 
			  (cdr (assoc pmml-compiling-mode 
				      '((play . playing) 
					(record . recording)))))
		    (pmml-update-mode-line)
		    (setq play-success t)))
	      (if (memq pmml-compiling-mode '(showtrk play record))
		  (progn
		    ;; We want to call pop-to-buffer here, but we can't,
		    ;; because the first character inputted from keyboard 
		    ;; goes to the old window.
		    (set-window-buffer (get-buffer-window buf)
				       pmml-track-buffer)))
	      )
	  (progn 
	    (set-buffer obuffer)
	    (or play-success (pmml-cleanup))))))))

(defun pmml-player-filter (process string)
  ;; dispose any output
  )

(defun pmml-player-sentinel (process event)
  (let ((obuffer (current-buffer)))
    (unwind-protect
	(progn
	  (set-buffer (process-buffer process))
	  (pmml-cleanup)
	  (setq pmml-status 'stopped)
	  (pmml-update-mode-line))
      (set-buffer obuffer))))

(defun pmml-do-inqure-record-param ()
  (let (ans)
    (while (progn
	     (setq ans (read-from-minibuffer
			"Use a metronome (yes or no) "
			(if pmml-record-with-metronome "yes" "no")))
	     (null (string-match "^\\(yes\\|no\\)$" ans)))
      (ding))
    (setq pmml-record-with-metronome (string= ans "yes"))
    (while (progn
	     (setq ans (read-from-minibuffer 
			"How many measures for preroll? " 
			(int-to-string pmml-record-preroll))) 
	     (null (string-match "^[0-9]+$" ans)))
      (ding))
    (setq pmml-record-preroll (string-to-int ans))))

(defun get-midi-fname (pmml-fname)
  (if (string-match "/[^/.]+\\.\\([^/.]*\\)$" pmml-fname)
      (concat (substring pmml-fname 0 (match-beginning 1)) "mid")
    (concat pmml-fname ".mid")))
  
(defun pmml-update-mode-line ()
  (set-buffer-modified-p (buffer-modified-p)))
	 
(defun pmml-start-server ()
  "Start the server process if it's not run yet."
  (if (and pmml-server-process 
	   (eq (process-status pmml-server-process) 'run))
      nil
    (if pmml-server-process 
	(delete-process pmml-server-process))
    (condition-case () 
	(setq pmml-server-process (eval (append (list 'start-process 
						      "pieceserver" 
						      nil)
						pmml-server-command)))
      (error nil))
    (if (or (null pmml-server-process)
	    (not (eq (process-status pmml-server-process) 'run)))
	(progn
	  (setq pmml-use-server nil)
	  (message "Could not run pieceserver.  Recording will be disabled."))
      (process-send-string pmml-server-process 
			   (format "D %d\n" pmml-chord-time-tolerance))
      (set-process-filter pmml-server-process 'pmml-server-filter))))
	  
(defun pmml-play-region-solo ()
  "Play the song within the current region using a temporary file.
It plays only the track on which the region beginning point is located." 
  (interactive)
  (pmml-play-region (region-beginning) (region-end) t))

(defun pmml-play-from-here-solo ()
  "Play the song from the current point using a temporary file.
It plays only the track on which the current point is located." 
  (interactive)
  (pmml-play-region (point) nil t))

(defun pmml-play-region-all ()
  "Play the song within the current region using a temporary file.
It plays all the tracks marked as `play' as well as the track 
on which the region beginning point is located."
  (interactive)
  (pmml-play-region (region-beginning) (region-end)))

(defun pmml-play-from-here-all ()
  "Play the song from the current point using a temporary file.
It plays all the tracks marked as `play' as well as the track 
on which the current point is located."
  (interactive)
  (pmml-play-region (point) nil))

(defun pmml-record ()
  "Record performance and insert its PMML code at the current point 
in parallel with playing the song from the current point."
  (interactive)
  (if (not pmml-use-server)
      (error "Recording is not supported."))
  (setq pmml-record-marker (point-marker))
  (if pmml-inqure-record-param
      (pmml-do-inqure-record-param))
  (pmml-play-region (point) nil nil t))

(defun pmml-record-finish ()
  "Finish recording and insert PMML code."
  (interactive)
  (if (eq pmml-status 'recording)
      (let ((mid-file (get-midi-fname pmml-temp-file)))
	(pmml-stop)
	(unwind-protect
	    (progn
	      (process-send-string pmml-server-process
				   (format "W %s\n" mid-file))
	      (save-excursion
		(set-buffer (marker-buffer pmml-record-marker))
		(goto-char pmml-record-marker)
		(call-process pmml-compiler-command nil t nil 
			      "-i" "rectrans" "-F" "ifilter" "-l" mid-file)
		(let ((endpt (point)))
		  (goto-char pmml-record-marker)
		  (pmml-indent-line)
		  (forward-line)
		  (if (< (point) endpt)
		      (progn
			(pmml-indent-line)
			(indent-region (point) endpt (current-indentation))))))
	      (pmml-cleanup))
	  (pmml-cleanup)))))

(defun pmml-play-region (beg end &optional solo record)
  "Play the song between beg and end using a temporary file.  end can be nil."
  (pmml-really-stop)
  ;; If region boundary is not on a token boundary, correct it.
  (save-excursion
    (goto-char beg)
    (or (memq (following-char) '(0 ?\n ?\  ?\t))
	(skip-chars-backward "^ \t\n"))
    ;; Is it on the beginning of a thread switch command?
    (if (looking-at 
	 ;; the move the beginning point after '{'
	 (cond ((boundp 'MULE) "\\([ \t\n]*\\(\\(\\sw\\|\\s_\\|\\Ca\\|:\\)*\\|newtrack[ \t\n]*([^\)]*)\\)[ \t\n]*{\\)")
	       ((boundp 'NEMACS) "\\([ \t\n]*\\(\\(\\sw\\|\\s_\\|\\z\\|:\\)*\\|newtrack[ \t\n]*([^\)]*)\\)[ \t\n]*{\\)")
	       (t "\\([ \t\n]*\\(\\(\\sw\\|\\s_\\|:\\)*\\|newtrack[ \t\n]*([^\)]*)\\)[ \t\n]*{\\)")))
	(goto-char (match-end 1)))
    (setq beg (point))
    (if end
	(progn
	  (goto-char end)
	  (or (memq (following-char) '(0 ?\n ?\  ?\t))
	      (skip-chars-backward "^ \t\n"))
	  (if (= end (point))
	      ;; if the end point is just after "}" "}+" or "}&",
	      ;; move the end point before it.
	      (progn 
		(skip-chars-backward " \t\n")
		(if (memq (preceding-char) '(?+ ?&)) (backward-char))
		(if (= (preceding-char) ?})
		    (backward-char)
		  (goto-char end))))
	  (setq end (point))
	  (if (> beg end) (setq beg end)))))
    
  ;; create a temp file.
  (let ((success nil)
	(temp-buffer (generate-new-buffer "*pmml-temp*"))
	(temp-file-name (concat pmml-temp-file ".pml")))
    (unwind-protect
	(progn
	  (write-region 1 beg temp-file-name nil 0)
	  (save-excursion 
	    (set-buffer temp-buffer)
	    (beginning-of-buffer)
	    (insert " mark(\".begin\") trackname(\".play\") ")
	    (write-region 1 (point) temp-file-name t 0))
	  (if (null end)
	      (write-region beg (1+ (buffer-size)) temp-file-name t 0)
	    (write-region beg end temp-file-name t 0)
	    (save-excursion 
	      (set-buffer temp-buffer)
	      (beginning-of-buffer)
	      (insert " mark(\".end\") ")
	      (write-region 1 (point) temp-file-name t 0))
	    (write-region end (1+ (buffer-size)) temp-file-name t 0))
	  (pmml-compile (concat (if (and record (/= pmml-record-preroll 0))
				    (format "-f/.begin/-%d " 
					    pmml-record-preroll)
				  "-f/.begin ")
				(if end "-t/.end ")
				(if solo "-T/.play"
				  (pmml-track-string pmml-play-tracks 
						     "/.play/")))
			temp-file-name 
			(if record 'record 'play))
	  (setq success t))
      (progn
	(kill-buffer temp-buffer)
	(or success 
	    (pmml-cleanup))))))

(defun pmml-show-track-summary ()
  "Show a summary for tracks."
  (interactive)
  (if (and (get-buffer "*pmml-track*")
	   (eq pmml-track-associated-buffer (current-buffer)))
      (switch-to-buffer-other-window pmml-track-buffer)
    (pmml-compile nil (buffer-file-name (current-buffer)) 'showtrk)))

(defun pmml-track-string (trks &optional extratrk)
  "Returns a string for -T option of pmml.  trks is an array of tracks
having same format as pmml-play-tracks.  extratrk is a string describing
an additional track to be played."
  (if (or pmml-use-server (null trks))
      nil
    (let* ((len (length trks))
	   (idx len)
	   (strlist nil)
	   idx1)
      (if (>= len 2)
	  (progn 
	    (setq strlist (list (format "%d-255" (1+ len))))
	    (while (>= (setq idx (1- idx)) 1) 
	      (setq idx1 idx)
	      (while (and (>= idx1 1)
			  (aref trks idx1))
		(setq idx1 (1- idx1)))
	      (cond ((and (zerop idx1)	; all tracks are t?
			  (= idx (1- len)) 
			  (null extratrk))
		     (setq strlist nil
			   idx 0))
		    ((>= (- idx idx1) 2)
		     (setq strlist (cons (concat (int-to-string (+ idx1 2)) 
						 "-"
						 (int-to-string (1+ idx)))
					 strlist))
		     (setq idx idx1))
		    ((>= (- idx idx1) 1)
		     (setq strlist (cons (int-to-string (1+ idx)) strlist)))))))
      (if (and (>= len 1) (null (aref trks 0)))
	  (setq strlist (cons "-1" strlist)))
      (if extratrk 
	  (setq strlist (cons extratrk strlist)))
      (and strlist 
	   (concat "-T"
		   (mapconcat (function (lambda(x) x)) strlist ","))))))

(defun pmml-update-tklist-in-server ()
  (if pmml-use-server
      (progn
	(process-send-string pmml-server-process "T\n")
	(if pmml-play-tracks
	    (process-send-string 
	     pmml-server-process
	     (concat "T " (mapconcat (function (lambda (x) (cond (x "0") 
								 (t "1"))))
				     pmml-play-tracks
				     " ")
		     "\n"))))))

;;
;; functions for step recording
;;
(defun pmml-server-filter (proc string)
  (let ((msglist (pmml-string-to-numlists string)))
    (mapcar 
     (function 
      (lambda (msg)
	(cond ((null msg) ; garbage?
	       nil)
	      ((= (car msg) ?D)   ; echoed note-on message?
	       (pmml-insert-notes))
	      ((= (car msg) ?I)   ; MIDI message?
	       (if (and (= (/ (nth 1 msg) 16) 9) 
			(/= (nth 3 msg) 0))  ; note-on message?
		   (setq pmml-note-list (cons msg pmml-note-list))))
	      ((= (car msg) ?A)   ; load/store acknowledge
	       (if (= (nth 1 msg) 1)  ; load acknowledge?
		   (pmml-cleanup)))
	      ((= (car msg) ?E)   ; load/store failure
	       (if (< (nth 1 msg) 3) 
		   (progn
		     (message "Loading MIDI file failed.")
		     (pmml-stop)
		     (pmml-cleanup))
		 (message "Writing MIDI file failed.")))
	      ((= (car msg) ?Z)   ; end of play
	       (setq pmml-status 'stopped)
	       (pmml-update-mode-line))
	      )))
     msglist)))

(defun pmml-insert-notes ()
  (if (and (boundp 'mybuffer) 
	   (eq pmml-status 'steprec)
	   (eq mybuffer (current-buffer))
	   pmml-note-list)
      (let (istring)
	(setq pmml-note-list 
	      (sort pmml-note-list (lambda (x y) (< (nth 2 x) (nth 2 y)))))
	(setq istring 
	      (concat 
	       (if (cdr pmml-note-list) "[")
	       (mapconcat (lambda (note)
			    (if pmml-show-velocity 
				(format "%s(v=%d)" 
					(pmml-noteno-to-str (nth 2 note))
					(nth 3 note))
			      (pmml-noteno-to-str (nth 2 note))))
			  pmml-note-list " ")
	       (if (cdr pmml-note-list) "]")))
	(if (string-match "[ \t\000\012({\[]" 
			  (char-to-string (preceding-char))) 
	    nil
	  (setq istring (concat " " istring)))
	;;(undo-boundary)
	(insert istring)
	(save-excursion
	  (let ((cur (point)))
	    (beginning-of-line)
	    (fill-individual-paragraphs (point) cur))) 
	(insert " ")
	;;(undo-start)  ;; undoing will not work properly without this
	))
  (setq pmml-note-list nil))

(defun pmml-noteno-to-str (note-number)
  (let ((octave (- (/ note-number 12) 1))
	(n (% note-number 12)))
    (concat (aref pmml-note-name n) (int-to-string octave))))

(defun pmml-string-to-numlists (string)
  (if (string-match "\\(\n\\)" string)
      (let ((line-end (match-end 1)))
	(cons (pmml-string-to-numlist 
	       (substring string 0 (- line-end 1)))
	      (pmml-string-to-numlists (substring string line-end))))))

(defun pmml-string-to-numlist (string)
  (cond ((string-match "\\([A-Za-z]+\\)" string)
	 (cons (string-to-char (substring string (match-beginning 1)))
	       (pmml-string-to-numlist (substring string (match-end 1)))))
	((string-match "\\([0-9]+\\)" string)
	 (cons (string-to-int string)
	       (pmml-string-to-numlist (substring string (match-end 1)))))))

;;
;; functions for indentation
;;
(defun pmml-indent-command ()
  "Indent current line as PMML code or insert a TAB character."
  (interactive)
  (if (and (not pmml-tab-always-indent)
	   (save-excursion
	     (skip-chars-backward " \t")
	     (not (bolp))))
      (insert-tab)
    (pmml-indent-line)))

(defun pmml-indent-line ()
  "Indent current line as PMML code."
  ;; This was originally c-indent-line in c-mode.el.
  (let ((indent (calculate-pmml-indent))
	beg shift-amt
	(pos (- (point-max) (point))))
    (beginning-of-line)
    (setq beg (point))
    (cond ((eq indent nil)
	   (setq indent (current-indentation)))
	  ((eq indent t)
	   (setq indent (calculate-c-indent-within-comment))))
    (skip-chars-forward " \t")
    (setq shift-amt (- indent (current-column)))
    (if (zerop shift-amt)
	(if (> (- (point-max) pos) (point))
	    (goto-char (- (point-max) pos)))
      (delete-region beg (point))
      (indent-to indent)
      ;; If initial point was within line's indentation,
      ;; position after the indentation.  Else stay at same point in text.
      (if (> (- (point-max) pos) (point))
	  (goto-char (- (point-max) pos))))
    shift-amt))

(defun pmml-beginning-of-defun ()
  "Move backward to next beginning of def, defeff or thread command."
  (and (re-search-backward "^\\s(\\|^[^ \t\n].*\\s(" nil 'move 1)
       (progn (beginning-of-line) t)))

(defun calculate-pmml-indent (&optional parse-start column-offset)
  "Return appropriate indentation for current line as PMML code.
Returns t if in a comment."
  (save-excursion
    (or column-offset (setq column-offset 0))
    (beginning-of-line)
    (let ((indent-point (point))
	  state 
	  openning-paren)
      (if parse-start
	  (goto-char parse-start)
	(pmml-beginning-of-defun)
	(setq parse-start (point)))
      (while (< (point) indent-point)
	(setq parse-start (point))
	(setq state (parse-partial-sexp (point) indent-point 0))
	(setq openning-paren (car (cdr state))))
      (cond ((nth 4 state)
	    ;; Inside a comment
	     t
	     )
	    ((eq (nth 3 state) ?\")
	     ;; In a string.  Don't change the indentation.
	     (current-indentation))
	    ((eq (nth 3 state) ?\')
	     ;; In a token-list constant.
	     (let ((qpos (pmml-find-last-single-quote 
			  parse-start indent-point))
		   cofs)
	       (save-excursion
		 (goto-char (1+ qpos))
		 (skip-chars-forward " \t")
		 (setq cofs (current-column)))
	       (calculate-pmml-indent (1+ qpos) cofs)))
	    ((null openning-paren)
	     ;; Line is at top level.
	     column-offset)
	    (t
	     ;; Line is at an intermediate level.
	     (goto-char indent-point)
	     (skip-chars-forward " \t")
	     (if (looking-at "\\s)")
		 ;; Closing } ] )
		 (progn 
		   (pmml-beginning-of-command state parse-start)
		   (max column-offset (current-indentation)))
	       (if (= (char-after openning-paren) ?\( ) 
		   ;; Inside ( .. )
		   (progn
		     (goto-char (1+ openning-paren))
		     (current-column))
		 ;; Inside { .. } or [ .. ]
		 (pmml-beginning-of-command state parse-start)
		 (+ (max column-offset (current-indentation))
		    pmml-indent-width))))))))

(defun pmml-find-last-single-quote (from to)
  "Find the last single quote in the region from..to."  
  (unwind-protect
      (progn
	(set-syntax-table pmml-alt-syntax-table)
	(car (cdr (parse-partial-sexp from to))))
    (set-syntax-table pmml-mode-syntax-table)))

(defun pmml-beginning-of-command (state parse-start)
  (goto-char openning-paren)
  (let (tstate)
    (while
	(progn
	  (beginning-of-line)
	  (setq tstate (parse-partial-sexp parse-start (point)))
	  (>= (car tstate) (car state)))
      (goto-char (car (cdr tstate))))))

(defun electric-pmml-paren (arg)
  "Insert character and correct line's indentation."
  (interactive "P")
  (if (and (not arg)
	   (eolp)
	   (save-excursion
	     (skip-chars-backward " \t")
	     (bolp)))
      (progn
	(insert last-command-char)
	(pmml-indent-line)
	(save-excursion (delete-char -1))))
  (self-insert-command (prefix-numeric-value arg)))

;;
;; functions for track-summary buffer
;;
(define-key pmml-track-mode-map "q" 'pmml-track-exit)
(define-key pmml-track-mode-map "j" 'forward-line)
(define-key pmml-track-mode-map "k" 'previous-line)
(define-key pmml-track-mode-map "p" 'pmml-track-set)
(define-key pmml-track-mode-map "n" 'pmml-track-unset)
(define-key pmml-track-mode-map " " 'pmml-track-toggle)
(define-key pmml-track-mode-map "a" 'pmml-track-all)
(define-key pmml-track-mode-map "s" 'pmml-track-solo)
(define-key pmml-track-mode-map "r" 'pmml-track-reextract)
(define-key pmml-track-mode-map "?" 'describe-mode)
(define-key pmml-track-mode-map "\C-m" 'forward-line)
(define-key pmml-track-mode-map "\C-c\C-f" 'pmml-track-play-file)
(define-key pmml-track-mode-map "\C-c\C-s" 'pmml-track-stop)

(if (>= pmml-emacs-version 19)
  (condition-case ()
    (progn
      (define-key pmml-track-mode-map [menu-bar] (make-sparse-keymap))
      (define-key pmml-track-mode-map [menu-bar pmml]
	(cons "PMML" (make-sparse-keymap "PMML")))
      
      (define-key pmml-track-mode-map [menu-bar pmml pmml-track-exit]
	'("Hide Track Summary" . pmml-track-exit))
      (define-key pmml-track-mode-map [menu-bar pmml pmml-track-toggle]
	'("Toggle Play Mark" . pmml-track-toggle))
      (define-key pmml-track-mode-map [menu-bar pmml pmml-track-play-file]
	'("Play from the Beginning" . pmml-track-play-file))
      (define-key pmml-track-mode-map [menu-bar pmml pmml-track-play-file]
	'("Play from the Beginning" . pmml-track-play-file))
      (define-key pmml-track-mode-map [menu-bar pmml pmml-track-stop]
	'("Stop" . pmml-track-stop)))
    (error nil)))

(defun pmml-track-mode ()
  "Major mode for displaying track infomration and selecting tracks to be 
played.  The \"P\" sign at the begining of each line means that the track 
is to be played.
Typed letters do not insert themselves; instead, they are commands:

 q   -- close track-summary window.
 p   -- mark track to be played.
 n   -- mark track not to be played.
 SPC -- toggle play mark.
 a   -- mark all tracks to be played.
 s   -- mark this track and track 1 to be played and mark other tracks 
        not to be played (solo).
 r   -- update track summary.  Track summary is re-extracted from the PMML 
        source code.
 ?   -- show this help.
 C-c C-f  -- play from the beginng (same as PMML mode).
 C-c C-s  -- stop (same as PMML mode)."
  (interactive)
  (kill-all-local-variables)
  (use-local-map pmml-track-mode-map)
  (setq major-mode 'pmml-track-mode)
  (setq mode-name "PMML Tracks")
  (setq buffer-read-only t)
  (run-hooks 'pmml-track-mode-hook))

(defun pmml-track-exit ()
  (interactive)
  (condition-case ()
      (replace-buffer-in-windows "*Help*")
    (error nil))
  (if (one-window-p) 
      (switch-to-buffer (other-buffer))
    (delete-window)))

(defun pmml-track-play-file ()
  (interactive)
  (condition-case ()
      (delete-window)
    (error nil))
  (switch-to-buffer pmml-track-associated-buffer)
  (pmml-play-file))

(defun pmml-track-stop ()
  (interactive)
  (save-excursion
    (set-buffer pmml-track-associated-buffer)
    (pmml-stop)))

(defun pmml-track-reextract ()
  (interactive)
  (condition-case ()
      (delete-window)
    (error nil))
  (switch-to-buffer pmml-track-associated-buffer)
  (setq pmml-track-associated-buffer nil)
  (pmml-show-track-summary))

(defun pmml-track-toggle ()
  (interactive)
  (let ((tk (pmml-track-get-trackid)))
    (save-excursion
      (set-buffer pmml-track-associated-buffer)
      (aset pmml-play-tracks (1- tk)
	    (not (aref pmml-play-tracks (1- tk))))
      (pmml-update-tklist-in-server)))
  (pmml-put-play-marks))

(defun pmml-track-set ()
  (interactive)
  (let ((tk (pmml-track-get-trackid)))
    (save-excursion
      (set-buffer pmml-track-associated-buffer)
      (aset pmml-play-tracks (1- tk) t)
      (pmml-update-tklist-in-server)))
  (pmml-put-play-marks)
  (forward-line))

(defun pmml-track-unset ()
  (interactive)
  (let ((tk (pmml-track-get-trackid)))
    (save-excursion
      (set-buffer pmml-track-associated-buffer)
      (aset pmml-play-tracks (1- tk) nil)
      (pmml-update-tklist-in-server)))
  (pmml-put-play-marks)
  (forward-line))

(defun pmml-track-all ()
  (interactive)
  (save-excursion
    (set-buffer pmml-track-associated-buffer)
    (fillarray pmml-play-tracks t)
    (pmml-update-tklist-in-server))
  (pmml-put-play-marks))

(defun pmml-track-solo ()
  (interactive)
  (let ((tk (pmml-track-get-trackid)))
    (save-excursion
      (set-buffer pmml-track-associated-buffer)
      (fillarray pmml-play-tracks nil)
      (aset pmml-play-tracks 0 t)
      (aset pmml-play-tracks (1- tk) t)
      (pmml-update-tklist-in-server)))
  (pmml-put-play-marks))

(defun pmml-track-get-trackid ()
  (beginning-of-line)
  (if (looking-at ". *\\([0-9]+\\)")
      (string-to-int 
       (buffer-substring (match-beginning 1) (match-end 1)))
    (error "Cursor not on a track.")))

(defun pmml-read-track-file (file buffer)
  "Insert the contents of track-summary file (standard output of the pmml 
compiler) to the track-summary buffer."
  (save-excursion
    (let (ntrks)
      (if (null (get-buffer "*pmml-track*"))
	  (let ((default-major-mode 'pmml-track-mode))
	    (setq pmml-track-buffer (generate-new-buffer "*pmml-track*"))))
      ;
      (set-buffer pmml-track-buffer)
      (let ((buffer-read-only nil))
	(erase-buffer)
	(insert (concat "File: " (buffer-file-name buffer) 
			"      Type ? for help.\n"))
	(insert-file-contents file)
	(setq pmml-track-associated-buffer buffer)
	(setq ntrks (pmml-put-play-marks)))
      ;
      (set-buffer buffer)   ; because pmml-play-tracks is buffer local
      (let ((otrks pmml-play-tracks)
	    (idx 0))
	(setq pmml-play-tracks (make-vector ntrks t))
	(while (and (< idx ntrks)
		    (< idx (length otrks)))
	  (aset pmml-play-tracks idx (aref otrks idx))
	  (setq idx (1+ idx)))))))

(defun pmml-put-play-marks ()
  "Insert play-track marks on the beginning of each line.
Returns the number of tracks."
  (save-excursion  ; to save the point location
    (let ((buffer-read-only nil)
	  (tk 1)
	  trks)
      (save-excursion  ; to get buffer-local pmml-play-tracks
	(set-buffer pmml-track-associated-buffer)
	(setq trks pmml-play-tracks))
      (goto-char 1)
      (forward-line 3)
      (while (looking-at ". *[0-9]")
	(delete-char 1)
	(if (or (null trks)
		(> tk (length trks))
		(aref trks (1- tk)))
	    (insert-char ?P 1)
	  (insert-char ?\  1))
	(forward-line 1)
	(setq tk (1+ tk)))
      (1- tk))))
