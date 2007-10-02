/* This file is part of the KDE libraries
 *  Copyright (c) 1997 Patrick Dowler <dowler@morgul.fsh.uvic.ca>
 *  Copyright (c) 2000 Dirk A. Mueller <mueller@kde.org>
 *  Copyright (c) 2002 Marc Mutz <mutz@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#ifndef K_NUMINPUT_H
#define K_NUMINPUT_H

#include <kdeui_export.h>

#include <QtGui/QWidget>
#include <QtGui/QSpinBox>

class QSlider;
class QSpinBox;
class QValidator;

class KIntSpinBox;
class KNumInputPrivate;

/**
 * You need to inherit from this class if you want to implement K*NumInput
 * for a different variable type
 *
 */
class KDEUI_EXPORT KNumInput : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( QString label READ label WRITE setLabel )
public:
    /**
     * Default constructor
     * @param parent If parent is 0, the new widget becomes a top-level
     * window. If parent is another widget, this widget becomes a child
     * window inside parent. The new widget is deleted when its parent is deleted.
     */
    explicit KNumInput(QWidget* parent=0);

    /**
     * @param below A pointer to another KNumInput.
     * @param parent parent widget
     * \deprecated - use the version without the below parameter instead
     */
    KDE_CONSTRUCTOR_DEPRECATED KNumInput(QWidget *parent, KNumInput* below);
    
    /**
     * Destructor
     */
    ~KNumInput();

    /**
     * Sets the text and alignment of the main description label.
     *
     * @param label The text of the label.
     *              Use QString() to remove an existing one.
     *
     * @param a The alignment of the label (Qt::Alignment).
     *          Default is @p Qt:AlignLeft | @p Qt:AlignTop.
     *
     * The vertical alignment flags have special meaning with this
     * widget:
     *
     *     @li @p Qt:AlignTop     The label is placed above the edit/slider
     *     @li @p Qt:AlignVCenter The label is placed left beside the edit
     *     @li @p Qt:AlignBottom  The label is placed below the edit/slider
     *
     */
    virtual void setLabel(const QString & label, Qt::Alignment a = Qt::AlignLeft | Qt::AlignTop);

    /**
     * @return the text of the label.
     */
    QString label() const;

    /**
     * @return if the num input has a slider.
     */
    bool showSlider() const;

    /**
     * Sets the spacing of tickmarks for the slider.
     *
     * @param minor Minor tickmark separation.
     * @param major Major tickmark separation.
     */
    void setSteps(int minor, int major);

    /**
     * Returns a size which fits the contents of the control.
     *
     * @return the preferred size necessary to show the control
     */
    virtual QSize sizeHint() const;

protected:
    /**
     * @return the slider widget.
     * @internal
     */
    QSlider *slider() const;
      
    /**
     * Call this function whenever you change something in the geometry
     * of your KNumInput child.
     *
     */
    void layout(bool deep);

    /**
     * You need to overwrite this method and implement your layout
     * calculations there.
     *
     * See KIntNumInput::doLayout and KDoubleNumInput::doLayout implementation
     * for details.
     *
     */
    virtual void doLayout() = 0;

private:
    friend class KNumInputPrivate;
    KNumInputPrivate * const d;
    
    Q_DISABLE_COPY(KNumInput)
};

/* ------------------------------------------------------------------------ */

/**
 * @short An input widget for integer numbers, consisting of a spinbox and a slider.
 *
 * KIntNumInput combines a QSpinBox and optionally a QSlider
 * with a label to make an easy to use control for setting some integer
 * parameter. This is especially nice for configuration dialogs,
 * which can have many such combinated controls.
 *
 * The slider is created only when the user specifies a range
 * for the control using the setRange function with the slider
 * parameter set to "true".
 *
 * A special feature of KIntNumInput, designed specifically for
 * the situation when there are several KIntNumInputs in a column,
 * is that you can specify what portion of the control is taken by the
 * QSpinBox (the remaining portion is used by the slider). This makes
 * it very simple to have all the sliders in a column be the same size.
 *
 * It uses KIntValidator validator class. KIntNumInput enforces the
 * value to be in the given range, and can display it in any base
 * between 2 and 36.
 *
 * \image html kintnuminput.png "KDE Int Number Input Spinbox"
 */

class KDEUI_EXPORT KIntNumInput : public KNumInput
{
    Q_OBJECT
    Q_PROPERTY( int value READ value WRITE setValue USER true )
    Q_PROPERTY( int minimum READ minimum WRITE setMinimum )
    Q_PROPERTY( int maximum READ maximum WRITE setMaximum )
    Q_PROPERTY( int referencePoint READ referencePoint WRITE setReferencePoint )
    Q_PROPERTY( double relativeValue READ relativeValue WRITE setRelativeValue )
    Q_PROPERTY( QString suffix READ suffix WRITE setSuffix )
    Q_PROPERTY( QString prefix READ prefix WRITE setPrefix )
    Q_PROPERTY( QString specialValueText READ specialValueText WRITE setSpecialValueText )

public:
    /**
     * Constructs an input control for integer values
     * with base 10 and initial value 0.
     */
    explicit KIntNumInput(QWidget *parent=0);
    /**
     * Constructor
     * It constructs a QSpinBox that allows the input of integer numbers
     * in the range of -INT_MAX to +INT_MAX. To set a descriptive label,
     * use setLabel(). To enforce the value being in a range and optionally to
     * attach a slider to it, use setRange().
     *
     * @param value  initial value for the control
     * @param base   numeric base used for display
     * @param parent parent QWidget
     */
    explicit KIntNumInput(int value, QWidget *parent=0,int base = 10);

    /**
     * Constructor
     *
     * the difference to the one above is the "below" parameter. It tells
     * this instance that it is visually put below some other KNumInput widget.
     * Note that these two KNumInput's need not to have the same parent widget
     * or be in the same layout group.
     * The effect is that it'll adjust it's layout in correspondence
     * with the layout of the other KNumInput's (you can build an arbitrary long
     * chain).
     *
     * @param below  append KIntNumInput to the KNumInput chain
     * @param value  initial value for the control
     * @param base   numeric base used for display
     * @param parent parent QWidget
     * 
     * \deprecated use the version without the below parameter instead.
     */
    KDE_CONSTRUCTOR_DEPRECATED KIntNumInput(KNumInput* below, int value, QWidget *parent, int base = 10);

    /**
     * Destructor
     *
     *
     */
    virtual ~KIntNumInput();

    /**
     * @return the current value.
     */
    int value() const;

    /**
     * @return the curent value in units of the referencePoint.
     */
    double relativeValue() const;

    /**
     * @return the current reference point
     */
    int referencePoint() const;

    /**
     * @return the suffix displayed behind the value.
     * @see setSuffix()
     */
    QString suffix() const;
    /**
     * @return the prefix displayed in front of the value.
     * @see setPrefix()
     */
    QString prefix() const;
    /**
     * @return the string displayed for a special value.
     * @see setSpecialValueText()
     */
    QString specialValueText() const;

    /**
     * @param min  minimum value
     * @param max  maximum value
     * @param step step size for the QSlider
     * @param slider whether the slider is created or not
     */
    void setRange(int min, int max, int step=1, bool slider=true);
    /**
     * Sets the minimum value.
     */
    void setMinimum(int min);
    /**
     * @return the minimum value.
     */
    int minimum() const;
    /**
     * Sets the maximum value.
     */
    void setMaximum(int max);
    /**
     * @return the maximum value.
     */
    int maximum() const;

    /**
     * Sets the special value text. If set, the SpinBox will display
     * this text instead of the numeric value whenever the current
     * value is equal to minVal(). Typically this is used for indicating
     * that the choice has a special (default) meaning.
     */
    void setSpecialValueText(const QString& text);

    virtual void setLabel(const QString & label, Qt::Alignment a = Qt::AlignLeft | Qt::AlignTop);

    /**
     * This method returns the minimum size necessary to display the
     * control. The minimum size is enough to show all the labels
     * in the current font (font change may invalidate the return value).
     *
     * @return the minimum size necessary to show the control
     */
    virtual QSize minimumSizeHint() const;
    
public Q_SLOTS:
    /**
     * Sets the value of the control.
     */
    void setValue(int);

    /**
     * Sets the value in units of the referencePoint
     */
    void setRelativeValue(double);

    /**
     * Sets the reference point for relativeValue.
     */
    void setReferencePoint(int);

    /**
     * Sets the suffix to @p suffix.
     * Use QString() to disable this feature.
     * Formatting has to be provided (e.g. a space separator between the
     * prepended @p value and the suffix's text has to be provided
     * as the first character in the suffix).
     *
     * @see QSpinBox::setSuffix(), #setPrefix()
     */
    void setSuffix(const QString &suffix);

    /**
     * Sets the prefix to @p prefix.
     * Use QString() to disable this feature.
     * Formatting has to be provided (see above).
     *
     * @see QSpinBox::setPrefix(), #setSuffix()
     */
    void setPrefix(const QString &prefix);

    /**
     * sets focus to the edit widget and marks all text in if mark == true
     *
     */
    void setEditFocus( bool mark = true );

Q_SIGNALS:
    /**
     * Emitted every time the value changes (by calling setValue() or
     * by user interaction).
     */
    void valueChanged(int);

    /**
     * Emitted whenever valueChanged is. Contains the change
     * relative to the referencePoint.
     */
    void relativeValueChanged(double);

private Q_SLOTS:
    void spinValueChanged(int);
    void slotEmitRelativeValueChanged(int);

protected:
    /**
     * @return the spin box widget.
     * @internal
     */
    QSpinBox *spinBox() const;
      
    virtual void doLayout();
    void resizeEvent ( QResizeEvent * );

private:
    void init(int value, int _base);

private:
    class KIntNumInputPrivate;
    friend class KIntNumInputPrivate;
    KIntNumInputPrivate * const d;
    
    Q_DISABLE_COPY(KIntNumInput)
};


/* ------------------------------------------------------------------------ */

class KDoubleLine;

/**
 * @short An input control for real numbers, consisting of a spinbox and a slider.
 *
 * KDoubleNumInput combines a QSpinBox and optionally a QSlider
 * with a label to make an easy to use control for setting some float
 * parameter. This is especially nice for configuration dialogs,
 * which can have many such combinated controls.
 *
 * The slider is created only when the user specifies a range
 * for the control using the setRange function with the slider
 * parameter set to "true".
 *
 * A special feature of KDoubleNumInput, designed specifically for
 * the situation when there are several instances in a column,
 * is that you can specify what portion of the control is taken by the
 * QSpinBox (the remaining portion is used by the slider). This makes
 * it very simple to have all the sliders in a column be the same size.
 *
 * It uses the KDoubleValidator validator class. KDoubleNumInput
 * enforces the value to be in the given range, but see the class
 * documentation of KDoubleSpinBox for the tricky
 * interrelationship of precision and values. All of what is said
 * there applies here, too.
 *
 * @see KIntNumInput, KDoubleSpinBox
 */

class KDEUI_EXPORT KDoubleNumInput : public KNumInput
{
    Q_OBJECT
    Q_PROPERTY( double value READ value WRITE setValue USER true )
    Q_PROPERTY( double minimum READ minimum WRITE setMinimum )
    Q_PROPERTY( double maximum READ maximum WRITE setMaximum )
    Q_PROPERTY( QString suffix READ suffix WRITE setSuffix )
    Q_PROPERTY( QString prefix READ prefix WRITE setPrefix )
    Q_PROPERTY( QString specialValueText READ specialValueText WRITE setSpecialValueText )
    Q_PROPERTY( int precision READ precision WRITE setPrecision )
    Q_PROPERTY( double referencePoint READ referencePoint WRITE setReferencePoint )
    Q_PROPERTY( double relativeValue READ relativeValue  WRITE setRelativeValue )

public:
    /**
     * Constructs an input control for double values
     * with initial value 0.00.
     */
    explicit KDoubleNumInput(QWidget *parent = 0);

    /**
     * Constructor
     *
     * @param lower lower boundary value
     * @param upper upper boundary value
     * @param value  initial value for the control
     * @param step   step size to use for up/down arrow clicks
     * @param precision number of digits after the decimal point
     * @param parent parent QWidget
     */
    KDoubleNumInput(double lower, double upper, double value, QWidget *parent=0,double step=0.01,
		    int precision=2);

    /**
     * destructor
     */
    virtual ~KDoubleNumInput();


    /**
     * Constructor
     *
     * the difference here is the "below" parameter. It tells this
     * instance that it is visually put below some other KNumInput
     * widget.  Note that these two KNumInput's need not to have the
     * same parent widget or be in the same layout group.  The effect
     * is that it'll adjust it's layout in correspondence with the
     * layout of the other KNumInput's (you can build an arbitrary long
     * chain).
     *
     * @param below  append KDoubleNumInput to the KDoubleNumInput chain
     * @param lower lower boundary value
     * @param upper upper boundary value
     * @param value  initial value for the control
     * @param step   step size to use for up/down arrow clicks
     * @param precision number of digits after the decimal point
     * @param parent parent QWidget
     *
     * \deprecated use the version without below instead
     */
    KDE_CONSTRUCTOR_DEPRECATED KDoubleNumInput(KNumInput* below,
		    double lower, double upper, double value, QWidget *parent=0,double step=0.02,
		    int precision=2);

    /**
     * @return the current value.
     */
    double value() const;

    /**
     * @return the suffix.
     * @see setSuffix()
     */
    QString suffix() const;

    /**
     * @return the prefix.
     * @see setPrefix()
     */
    QString prefix() const;

    /**
     * @return the precision.
     * @see setPrecision()
     */
    int precision() const;

    /**
     * @return the string displayed for a special value.
     * @see setSpecialValueText()
     */
    QString specialValueText() const;

     /**
     * @param min  minimum value
     * @param max  maximum value
     * @param step step size for the QSlider
     * @param slider whether the slider is created or not
     */
    void setRange(double min, double max, double step=1, bool slider=true);
    /**
     * Sets the minimum value.
     */
    void setMinimum(double min);
    /**
     * @return the minimum value.
     */
    double minimum() const;
    /**
     * Sets the maximum value.
     */
    void setMaximum(double max);
    /**
     * @return the maximum value.
     */
    double maximum() const;

    /**
     * Specifies the number of digits to use.
     */
    void setPrecision(int precision);

    /**
     * @return the reference point for relativeValue calculation
     */
    double referencePoint() const;

    /**
     * @return the current value in units of referencePoint.
     */
    double relativeValue() const;

    /**
     * Sets the special value text. If set, the spin box will display
     * this text instead of the numeric value whenever the current
     * value is equal to minVal(). Typically this is used for indicating
     * that the choice has a special (default) meaning.
     */
    void setSpecialValueText(const QString& text);

    virtual void setLabel(const QString & label, Qt::Alignment a = Qt::AlignLeft | Qt::AlignTop);
    virtual QSize minimumSizeHint() const;

public Q_SLOTS:
    /**
     * Sets the value of the control.
     */
    void setValue(double);

    /**
     * Sets the value in units of referencePoint.
     */
    void setRelativeValue(double);

    /**
     * Sets the reference Point to @p ref. It @p ref == 0, emitting of
     * relativeValueChanged is blocked and relativeValue
     * just returns 0.
     */
    void setReferencePoint(double ref);

    /**
     * Sets the suffix to be displayed to @p suffix. Use QString() to disable
     * this feature. Note that the suffix is attached to the value without any
     * spacing. So if you prefer to display a space separator, set suffix
     * to something like " cm".
     * @see setSuffix()
     */
    void setSuffix(const QString &suffix);

    /**
     * Sets the prefix to be displayed to @p prefix. Use QString() to disable
     * this feature. Note that the prefix is attached to the value without any
     * spacing.
     * @see setPrefix()
     */
    void setPrefix(const QString &prefix);

Q_SIGNALS:
    /**
     * Emitted every time the value changes (by calling setValue() or
     * by user interaction).
     */
    void valueChanged(double);
    /**
     * This is an overloaded member function, provided for
     * convenience. It essentially behaves like the above function.
     *
     * Contains the value in units of referencePoint.
     */
    void relativeValueChanged(double);

private Q_SLOTS:
    void sliderMoved(int);
    void slotEmitRelativeValueChanged(double);

protected:
    virtual void doLayout();
    void resizeEvent ( QResizeEvent * );

    friend class KDoubleLine;
private:
    void init(double value, double lower, double upper,
    double step, int precision);
    double mapSliderToSpin(int) const;
    void updateLegacyMembers();

private:
    class KDoubleNumInputPrivate;
    friend class KDoubleNumInputPrivate;
    KDoubleNumInputPrivate * const d;

    Q_DISABLE_COPY(KDoubleNumInput)
};


/* ------------------------------------------------------------------------ */

/**
 *  @short A QSpinBox with support for arbitrary base numbers.
 *
 *  A QSpinBox with support for arbitrary base numbers
 *  (e.g. hexadecimal).
 *
 *  The class provides an easy interface to use other
 *  numeric systems than the decimal.
 */
class KDEUI_EXPORT KIntSpinBox : public QSpinBox
{
    Q_OBJECT
    Q_PROPERTY( int base READ base WRITE setBase )

public:

    /**
     *  Constructor.
     *
     *  Constructs a widget with an integer inputline with a little scrollbar
     *  and a slider, with minimal value 0, maximal value 99, step 1, base 10
     *  and initial value 0.
     */
    explicit KIntSpinBox( QWidget *parent = 0 );

    /**
     *  Constructor.
     *
     *  Constructs a widget with an integer inputline with a little scrollbar
     *  and a slider.
     *
     *  @param lower  The lowest valid value.
     *  @param upper  The greatest valid value.
     *  @param step   The step size of the scrollbar.
     *  @param value  The actual value.
     *  @param base   The base of the used number system.
     *  @param parent The parent of the widget.
     */
    KIntSpinBox(int lower, int upper, int step, int value, QWidget *parent,int base = 10);

    /**
     *  Destructor.
     */
    virtual ~KIntSpinBox();

    /**
     * Sets the base in which the numbers in the spin box are represented.
     */
    void setBase(int base);
    /**
     * @return the base in which numbers in the spin box are represented.
     */
    int base() const;
    /**
     * sets focus and optionally marks all text
     *
     */
    void setEditFocus(bool mark);

protected:

    /**
     *  Overloaded the method in QSpinBox
     *  to make use of the base given in the constructor.
     */
    virtual QString textFromValue(int) const;

    /**
     *  Overloaded the method in QSpinBox
     *  to make use of the base given in the constructor.
     */
    virtual int valueFromText(const QString &text) const;

private:
    class KIntSpinBoxPrivate;
    friend class KIntSpinBoxPrivate;
    KIntSpinBoxPrivate *const d;
    
    Q_DISABLE_COPY(KIntSpinBox)
};


/* --------------------------------------------------------------------------- */

/**
   @short A spin box for fractional numbers.

   This class provides a spin box for fractional numbers.

   \image html kdoublespinbox.png "KDE Fractional Number Spinbox"

   See below for code examples on how to use this class.

   \b Parameters \n

   To make successful use of KDoubleSpinBox, you need to understand the
   relationship between precision and available range.

   @li precision: The number of digits after the decimal point.
   @li maximum/minimum: upper and lower bounds of the valid range
   @li lineStep: the size of the step that is made when the user hits
                 the up or down buttons

   Since we work with fixed-point numbers internally, the maximum
   precision is a function of the valid range and vice versa. More
   precisely, the following relationships hold:
   \code
   max( abs(minimum()), abs(maximum() ) <= INT_MAX/10^precision
   maxPrecision = floor( log10( INT_MAX/max(abs(minimum()),abs(maximum())) ) )
   \endcode

   Since the value, bounds and lineStep are rounded to the current
   precision, you may find that the order of setting these
   parameters matters. As an example, the following are @em not equivalent (try
   it!):

   \code
   // sets precision,
   // then min/max value (rounded to precision and clipped to obtainable range if needed)
   // then value and lineStep
   KDoubleSpinBox * spin = new KDoubleSpinBox( 0, 9.999, 0.001, 4.321, 3, this );

   // sets minimum to 0; maximum to 10.00(!); value to 4.32(!) and only then
   // increases the precision - too late, since e.g. value has already been rounded...
   KDoubleSpinBox * spin = new KDoubleSpinBox( this );
   spin->setMinimum( 0 );
   spin->setMaximum( 9.999 );
   spin->setValue( 4.321 );
   spin->setPrecision( 3 );
   \endcode

   @author Marc Mutz <mutz@kde.org>
**/

class KDEUI_EXPORT KDoubleSpinBox : public QSpinBox {
  Q_OBJECT
  Q_PROPERTY( bool acceptLocalizedNumbers READ acceptLocalizedNumbers WRITE setAcceptLocalizedNumbers )
  Q_OVERRIDE( double maximum READ maximum WRITE setMaximum )
  Q_OVERRIDE( double minimum READ minimum WRITE setMinimum )
  Q_OVERRIDE( double singleStep READ singleStep WRITE setSingleStep )
  Q_OVERRIDE( double value READ value WRITE setValue )
  Q_PROPERTY( int precision READ precision WRITE setPrecision )

public:
  /** Constructs a KDoubleSpinBox with parent @p parent and
      default values for range and value (whatever QRangeControl
      uses) and precision (2). */
  explicit KDoubleSpinBox( QWidget * parent = 0 );

  /** Constructs a KDoubleSpinBox with parent @p parent, range
      [ @p lower, @p upper ], lineStep @p step, precision @p
      precision and initial value @p value. */
  KDoubleSpinBox( double lower, double upper, double step, double value,
		  QWidget *parent,int precision=2);

  virtual ~KDoubleSpinBox();

  /** @return whether the spinbox uses localized numbers */
  bool acceptLocalizedNumbers() const;

  /** Sets whether to use and accept localized numbers as returned by
      KLocale::formatNumber() */
  virtual void setAcceptLocalizedNumbers( bool accept );

  /** Sets a new range for the spin box values. Note that @p lower, @p
      upper and @p step are rounded to @p precision decimal points
      first. */
  void setRange( double lower, double upper, double step=0.01, int precision=2 );

  /** @return the current number of digits displayed to the right of the
      decimal point. */
  int precision() const;

  /** Equivalent to setPrecision( @p precision, @p false ); Needed
      since Qt's moc doesn't ignore trailing parameters with default
      args when searching for a property setter method. */
  void setPrecision( int precision );

  /** Sets the precision (number of digits to the right of the decimal point). Note
      that there is a tradeoff between the precision used and the available range of
      values. See the class documentation above for more information on this.

      @param precision the new precision to use

      @param force if true, disables checking of bounds violations that can
             arise if you increase the precision so much that the
             minimum and maximum values can't be represented
             anymore. Disabling is useful if you were going to disable range
             control in any case.
  **/
  virtual void setPrecision( int precision, bool force );

  /** @return the current value */
  double value() const;

  /** @return the current lower bound */
  double minimum() const;

  /** Sets the lower bound of the range to @p value, subject to the
      contraints that @p value is first rounded to the current
      precision and then clipped to the maximum range interval that can
      be handled at that precision.
      @see maximum, minimum, setMaximum, setRange
  */
  void setMinimum( double value );

  /** @return the current upper bound */
  double maximum() const;

  /** Sets the upper bound of the range to @p value, subject to the
      contraints that @p value is first rounded to the current
      precision and then clipped to the maximum range interval
      that can be handled at that precision.
      @see minimum, maximum, setMinimum, setRange
  */
  void setMaximum( double value );

  /** @return the current step size */
  double singleStep() const;

  /** Sets the step size for clicking the up/down buttons to @p step,
      subject to the constraints that @p step is first rounded to the
      current precision and then clipped to the meaningful interval
      [ 1, @p maximum() - @p minimum() ]. */
  void setSingleStep( double step );

  /** Overridden to ignore any setValidator() calls. */
  void setValidator( const QValidator * );

Q_SIGNALS:
  /** Emitted whenever QSpinBox::valueChanged( int ) is emitted. */
  void valueChanged( double value );

public Q_SLOTS:
  /** Sets the current value to @p value, subject to the constraints
      that @p value is first rounded to the current precision and then
      clipped to the interval [ @p minimum() , @p maximum() ]. */
  virtual void setValue( double value );

protected:
  virtual QString textFromValue(int) const;
  virtual int valueFromText(const QString &text) const;

protected Q_SLOTS:
  void slotValueChanged( int value );

private:
  typedef QSpinBox base;
  void updateValidator();
  int maxPrecision() const;

  class Private;
  Private * const d;
};

#endif // K_NUMINPUT_H
