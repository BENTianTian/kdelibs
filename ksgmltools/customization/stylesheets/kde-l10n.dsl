;; -*- dsssl -*-
;; This file is part of the KDE application ksgmltools
;; For licensing, documentation etc, refer to that application
;; This file should not be distributed outside ksgmltools

; it would be a nice addition to be able to say: it pt-BR not found, use pt
; I guess that's up to ($lang$).

; This variable can be interpreted in many ways: a URL to
; - The official KDE web site
; - The translation of the official KDE web site
; - The site of the translation team
; - A local copy
; - ...
; My strong preference goes to the second choice: the translated official
; web site.  That gives the user access in his/her own language and it
; provides links to localisation and translation teams etc etc, all in one go.
; If there was no information on a site, then I chose the English version.

(define (kde-gentext-home-url)
  (case ($lang$)
    <![%kde-l10n-ca[ (("ca") %kde-gentext-ca-home-url%) ]]>
    <![%kde-l10n-cs[ (("cs") %kde-gentext-cs-home-url%) ]]>
    <![%kde-l10n-da[ (("da") %kde-gentext-da-home-url%) ]]>
    <![%kde-l10n-de[ (("de") %kde-gentext-de-home-url%) ]]>
    <![%kde-l10n-el[ (("el") %kde-gentext-el-home-url%) ]]>
    <![%kde-l10n-en[ (("en") %kde-gentext-en-home-url%) ]]>
    <![%kde-l10n-es[ (("es") %kde-gentext-es-home-url%) ]]>
    <![%kde-l10n-et[ (("et") %kde-gentext-et-home-url%) ]]>
    <![%kde-l10n-fi[ (("fi") %kde-gentext-fi-home-url%) ]]>
    <![%kde-l10n-fr[ (("fr") %kde-gentext-fr-home-url%) ]]>
    <![%kde-l10n-it[ (("it") %kde-gentext-it-home-url%) ]]>
    <![%kde-l10n-ja[ (("ja") %kde-gentext-ja-home-url%) ]]>
    <![%kde-l10n-nl[ (("nl") %kde-gentext-nl-home-url%) ]]>
    <![%kde-l10n-no[ (("no") %kde-gentext-no-home-url%) ]]>
    <![%kde-l10n-pl[ (("pl") %kde-gentext-pl-home-url%) ]]>
    <![%kde-l10n-pt[ (("pt") %kde-gentext-pt-home-url%) ]]>
    <![%kde-l10n-pt-BR[ (("pt-br") %kde-gentext-pt-BR-home-url%) ]]>
    <![%kde-l10n-ro[ (("ro") %kde-gentext-ro-home-url%) ]]>
    <![%kde-l10n-ru[ (("ru") %kde-gentext-ru-home-url%) ]]>
    <![%kde-l10n-sk[ (("sk") %kde-gentext-sk-home-url%) ]]>
    <![%kde-l10n-sl[ (("sl") %kde-gentext-sl-home-url%) ]]>
    <![%kde-l10n-sr[ (("sr") %kde-gentext-sr-home-url%) ]]>
    <![%kde-l10n-sv[ (("sv") %kde-gentext-sv-home-url%) ]]>
    <![%kde-l10n-zh-CN[ (("zh-cn") %kde-gentext-zh-CN-home-url%) ]]>
    (else (error "L10N ERROR: kde-gentext-logo-alt: no localised version"))))

; split up the in files for different languages - take over system from DB

(define (kde-gentext-logo-alt)
  (case ($lang$)
    <!-- <![%kde-l10n-ca[ (("ca") %kde-gentext-ca-logo-alt%) ]]> -->
    <!-- <![%kde-l10n-cs[ (("cs") %kde-gentext-cs-logo-alt%) ]]> -->
    <![%kde-l10n-da[ (("da") %kde-gentext-da-logo-alt%) ]]>
    <![%kde-l10n-de[ (("de") %kde-gentext-de-logo-alt%) ]]>
    <!-- <![%kde-l10n-el[ (("el") %kde-gentext-el-logo-alt%) ]]> -->
    <![%kde-l10n-en[ (("en") %kde-gentext-en-logo-alt%) ]]>
    <![%kde-l10n-es[ (("es") %kde-gentext-es-logo-alt%) ]]>
    <![%kde-l10n-et[ (("et") %kde-gentext-et-logo-alt%) ]]>
    <![%kde-l10n-fi[ (("fi") %kde-gentext-fi-logo-alt%) ]]>
    <![%kde-l10n-fr[ (("fr") %kde-gentext-fr-logo-alt%) ]]>
    <![%kde-l10n-it[ (("it") %kde-gentext-it-logo-alt%) ]]>
    <![%kde-l10n-ja[ (("ja") %kde-gentext-ja-logo-alt%) ]]>
    <![%kde-l10n-nl[ (("nl") %kde-gentext-nl-logo-alt%) ]]>
    <![%kde-l10n-no[ (("no") %kde-gentext-no-logo-alt%) ]]>
    <![%kde-l10n-pl[ (("pl") %kde-gentext-pl-logo-alt%) ]]>
    <![%kde-l10n-pt[ (("pt") %kde-gentext-pt-logo-alt%) ]]>
    <![%kde-l10n-pt-BR[ (("pt-br") %kde-gentext-pt-BR-logo-alt%) ]]>
    <![%kde-l10n-ro[ (("ro") %kde-gentext-ro-logo-alt%) ]]>
    <![%kde-l10n-ru[ (("ru") %kde-gentext-ru-logo-alt%) ]]>
    <!-- <![%kde-l10n-sk[ (("sk") %kde-gentext-sk-logo-alt%) ]]> -->
    <![%kde-l10n-sl[ (("sl") %kde-gentext-sl-logo-alt%) ]]>
    <!-- <![%kde-l10n-sr[ (("sr") %kde-gentext-sr-logo-alt%) ]]> -->
    <![%kde-l10n-sv[ (("sv") %kde-gentext-sv-logo-alt%) ]]>
    <![%kde-l10n-zh-CN[ (("zh-cn") %kde-gentext-zh-CN-logo-alt%) ]]>
    (else (error "L10N ERROR: kde-gentext-logo-alt: no localised version"))))

;; In future, the charset in the content-type should become charset=utf-8
;; for all documents.
;; Concerning language codes: RFC1766 gives hyphen and not the underscore
;; as specifier for countries (which are by strong recommendation in upper 
;; case).
(define (kde-gentext-html-header-tags)
  (append
   (list 
    (list "META"
	  (list "HTTP-EQUIV" "Content-Type")
	  (list "CONTENT" (string-append "text/html; charset="
					 (kde-gentext-encoding)))))
   (case ($lang$)
    <![%kde-l10n-ca[ (("ca") %kde-gentext-ca-html-header-tags%) ]]>
    <![%kde-l10n-cs[ (("cs") %kde-gentext-cs-html-header-tags%) ]]>
    <![%kde-l10n-da[ (("da") %kde-gentext-da-html-header-tags%) ]]>
    <![%kde-l10n-de[ (("de") %kde-gentext-de-html-header-tags%) ]]>	    
    <![%kde-l10n-el[ (("el") %kde-gentext-el-html-header-tags%) ]]>	    
    <![%kde-l10n-en[ (("en") %kde-gentext-en-html-header-tags%) ]]>	    
    <![%kde-l10n-es[ (("es") %kde-gentext-es-html-header-tags%) ]]>
    <![%kde-l10n-et[ (("et") %kde-gentext-et-html-header-tags%) ]]>
    <![%kde-l10n-fi[ (("fi") %kde-gentext-fi-html-header-tags%) ]]>
    <![%kde-l10n-fr[ (("fr") %kde-gentext-fr-html-header-tags%) ]]>	    
    <![%kde-l10n-it[ (("it") %kde-gentext-it-html-header-tags%) ]]>
    <![%kde-l10n-ja[ (("ja") %kde-gentext-ja-html-header-tags%) ]]>	    
    <![%kde-l10n-nl[ (("nl") %kde-gentext-nl-html-header-tags%) ]]>	    
    <![%kde-l10n-no[ (("no") %kde-gentext-no-html-header-tags%) ]]>	    
    <![%kde-l10n-pl[ (("pl") %kde-gentext-pl-html-header-tags%) ]]>
    <![%kde-l10n-pt[ (("pt") %kde-gentext-pt-html-header-tags%) ]]>
    <![%kde-l10n-pt-BR[ (("pt-br") %kde-gentext-pt-BR-html-header-tags%) ]]>
    <![%kde-l10n-ro[ (("ro") %kde-gentext-ro-html-header-tags%) ]]>
    <![%kde-l10n-ru[ (("ru") %kde-gentext-ru-html-header-tags%) ]]>
    <![%kde-l10n-sk[ (("sk") %kde-gentext-sk-html-header-tags%) ]]>
    <![%kde-l10n-sl[ (("sl") %kde-gentext-sl-html-header-tags%) ]]>
    <![%kde-l10n-sr[ (("sr") %kde-gentext-sr-html-header-tags%) ]]>
    <![%kde-l10n-sv[ (("sv") %kde-gentext-sv-html-header-tags%) ]]>
    <![%kde-l10n-zh-CN[ (("zh-cn") %kde-gentext-zh-CN-html-header-tags%) ]]>
    (else (error "L10N ERROR: kde-gentext-html-header-tags: no localised version")))
   (list
    (list "META"
	  (list "HTTP-EQUIV" "Content-Language")
	  (list "CONTENT" ($lang$)))
    ; for localised style sheet
    (kde-external-ss (string-append %kde-doc-common-path% "kde-localised.css")
		     (string-append "KDE-" (kde-gentext-language)))
    ; for KDE-default style sheet (must come after localised!)
    (kde-external-ss (string-append %kde-doc-common-path% "kde-default.css")
		     (string-append "KDE-" (kde-gentext-default))))))


(define (kde-gentext-encoding)
  (case ($lang$)
    <![%kde-l10n-ca[ (("ca") %kde-gentext-ca-encoding%) ]]>
    <![%kde-l10n-cs[ (("cs") %kde-gentext-cs-encoding%) ]]>
    <![%kde-l10n-da[ (("da") %kde-gentext-da-encoding%) ]]>
    <![%kde-l10n-de[ (("de") %kde-gentext-de-encoding%) ]]>
    <![%kde-l10n-el[ (("el") %kde-gentext-el-encoding%) ]]>
    <![%kde-l10n-en[ (("en") %kde-gentext-en-encoding%) ]]>
    <![%kde-l10n-es[ (("es") %kde-gentext-es-encoding%) ]]>
    <![%kde-l10n-et[ (("et") %kde-gentext-et-encoding%) ]]>
    <![%kde-l10n-fi[ (("fi") %kde-gentext-fi-encoding%) ]]>
    <![%kde-l10n-fr[ (("fr") %kde-gentext-fr-encoding%) ]]>
    <![%kde-l10n-it[ (("it") %kde-gentext-it-encoding%) ]]>
    <![%kde-l10n-ja[ (("ja") %kde-gentext-ja-encoding%) ]]>
    <![%kde-l10n-nl[ (("nl") %kde-gentext-nl-encoding%) ]]>
    <![%kde-l10n-no[ (("no") %kde-gentext-no-encoding%) ]]>
    <![%kde-l10n-pl[ (("pl") %kde-gentext-pl-encoding%) ]]>
    <![%kde-l10n-pt[ (("pt") %kde-gentext-pt-encoding%) ]]>
    <![%kde-l10n-pt-BR[ (("pt-br") %kde-gentext-pt-BR-encoding%) ]]>
    <![%kde-l10n-ro[ (("ro") %kde-gentext-ro-encoding%) ]]>
    <![%kde-l10n-ru[ (("ru") %kde-gentext-ru-encoding%) ]]>
    <![%kde-l10n-sk[ (("sk") %kde-gentext-sk-encoding%) ]]>
    <![%kde-l10n-sl[ (("sl") %kde-gentext-sl-encoding%) ]]>
    <![%kde-l10n-sr[ (("sr") %kde-gentext-sr-encoding%) ]]>
    <![%kde-l10n-sv[ (("sv") %kde-gentext-sv-encoding%) ]]>
    <![%kde-l10n-zh-CN[ (("zh-cn") %kde-gentext-zh-CN-encoding%) ]]>
    (else (error "L10N ERROR: kde-gentext-encoding: no localised version"))))

(define (kde-gentext-default)
  (case ($lang$)
    <![%kde-l10n-ca[ (("ca") %kde-gentext-ca-default%) ]]>
    <![%kde-l10n-cs[ (("cs") %kde-gentext-cs-default%) ]]>
    <![%kde-l10n-da[ (("da") %kde-gentext-da-default%) ]]>
    <![%kde-l10n-de[ (("de") %kde-gentext-de-default%) ]]>
    <![%kde-l10n-el[ (("el") %kde-gentext-el-default%) ]]>
    <![%kde-l10n-en[ (("en") %kde-gentext-en-default%) ]]>
    <![%kde-l10n-es[ (("es") %kde-gentext-es-default%) ]]>
    <![%kde-l10n-et[ (("et") %kde-gentext-et-default%) ]]>
    <![%kde-l10n-fi[ (("fi") %kde-gentext-fi-default%) ]]>
    <![%kde-l10n-fr[ (("fr") %kde-gentext-fr-default%) ]]>
    <![%kde-l10n-it[ (("it") %kde-gentext-it-default%) ]]>
    <![%kde-l10n-ja[ (("ja") %kde-gentext-ja-default%) ]]>
    <![%kde-l10n-nl[ (("nl") %kde-gentext-nl-default%) ]]>
    <![%kde-l10n-no[ (("no") %kde-gentext-no-default%) ]]>
    <![%kde-l10n-pl[ (("pl") %kde-gentext-pl-default%) ]]>
    <![%kde-l10n-pt[ (("pt") %kde-gentext-pt-default%) ]]>
    <![%kde-l10n-pt-BR[ (("pt-br") %kde-gentext-pt-BR-default%) ]]>
    <![%kde-l10n-ro[ (("ro") %kde-gentext-ro-default%) ]]>
    <![%kde-l10n-ru[ (("ru") %kde-gentext-ru-default%) ]]>
    <![%kde-l10n-sk[ (("sk") %kde-gentext-sk-default%) ]]>
    <![%kde-l10n-sl[ (("sl") %kde-gentext-sl-default%) ]]>
    <![%kde-l10n-sr[ (("sr") %kde-gentext-sr-default%) ]]>
    <![%kde-l10n-sv[ (("sv") %kde-gentext-sv-default%) ]]>
    <![%kde-l10n-zh-CN[ (("zh-cn") %kde-gentext-zh-CN-default%) ]]>
    (else (error "L10N ERROR: kde-gentext-default: no localised version"))))

(define (kde-gentext-language)
  (case ($lang$)
    <![%kde-l10n-ca[ (("ca") %kde-gentext-ca-language%) ]]>
    <![%kde-l10n-cs[ (("cs") %kde-gentext-cs-language%) ]]>
    <![%kde-l10n-da[ (("da") %kde-gentext-da-language%) ]]>
    <![%kde-l10n-de[ (("de") %kde-gentext-de-language%) ]]>
    <![%kde-l10n-el[ (("el") %kde-gentext-el-language%) ]]>
    <![%kde-l10n-en[ (("en") %kde-gentext-en-language%) ]]>
    <![%kde-l10n-es[ (("es") %kde-gentext-es-language%) ]]>
    <![%kde-l10n-et[ (("et") %kde-gentext-et-language%) ]]>
    <![%kde-l10n-fi[ (("fi") %kde-gentext-fi-language%) ]]>
    <![%kde-l10n-fr[ (("fr") %kde-gentext-fr-language%) ]]>
    <![%kde-l10n-it[ (("it") %kde-gentext-it-language%) ]]>
    <![%kde-l10n-ja[ (("ja") %kde-gentext-ja-language%) ]]>
    <![%kde-l10n-nl[ (("nl") %kde-gentext-nl-language%) ]]>
    <![%kde-l10n-no[ (("no") %kde-gentext-no-language%) ]]>
    <![%kde-l10n-pl[ (("pl") %kde-gentext-pl-language%) ]]>
    <![%kde-l10n-pt[ (("pt") %kde-gentext-pt-language%) ]]>
    <![%kde-l10n-pt-BR[ (("pt-br") %kde-gentext-pt-BR-language%) ]]>
    <![%kde-l10n-ro[ (("ro") %kde-gentext-ro-language%) ]]>
    <![%kde-l10n-ru[ (("ru") %kde-gentext-ru-language%) ]]>
    <![%kde-l10n-sk[ (("sk") %kde-gentext-sk-language%) ]]>
    <![%kde-l10n-sl[ (("sl") %kde-gentext-sl-language%) ]]>
    <![%kde-l10n-sr[ (("sr") %kde-gentext-sr-language%) ]]>
    <![%kde-l10n-sv[ (("sv") %kde-gentext-sv-language%) ]]>
    <![%kde-l10n-zh-CN[ (("zh-cn") %kde-gentext-zh-CN-language%) ]]>
    (else (error "L10N ERROR: kde-gentext-language: no localised version"))))
