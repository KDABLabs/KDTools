#include <Cocoa/Cocoa.h>

#include <QMacCocoaViewContainer>

class HelpButtonViewContainer : public QMacCocoaViewContainer
{
public:
    HelpButtonViewContainer( void* widget, QWidget* parent = 0 )
        : QMacCocoaViewContainer( widget, parent )
    {
    }

    void clicked()
    {
        QMetaObject::invokeMethod( parent(), "click" );
    }

    QSize sizeHint() const
    {
        return QSize( 23, 23 ); // this is the only size the button can have
    }
};


@interface NSButtonTarget: NSObject
{
@public
    HelpButtonViewContainer *target;
}
-(id)initWithObject:(HelpButtonViewContainer*)object;
-(IBAction)clicked:(id)sender;
@end
@implementation NSButtonTarget
-(id)initWithObject:(HelpButtonViewContainer*)object
{
    self = [super init];
    target = object;
    return self;
}
-(IBAction)clicked:(id)sender
{
    Q_UNUSED(sender)
    target->clicked();
}
@end


/*!
  Creates a NSButton with help button style.
  \internal
  */
#include <QDebug>
QMacCocoaViewContainer* createNSHelpButton( QWidget* parent )
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    Q_UNUSED( pool )

    HelpButtonViewContainer* const container = new HelpButtonViewContainer( 0, parent );

    NSButton *button = [[NSButton alloc] init];
    [button setTitle: @""];
    [button setBezelStyle: NSHelpButtonBezelStyle];

    NSButtonTarget *bt = [[NSButtonTarget alloc] init];
    bt->target = container;
    [button setTarget: bt];
    [button setAction: @selector(clicked:)];

    qDebug() << "setCocoaView";
    container->setCocoaView( button );

    return container;
}
