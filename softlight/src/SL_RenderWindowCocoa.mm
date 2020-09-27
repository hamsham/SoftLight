
#import <AppKit/AppKit.h>
#import <Cocoa/Cocoa.h>

#include <pthread.h> // pthread_mutex_t, pthread_mutex_lock()

#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/Log.h"

#include "softlight/SL_Color.hpp"
#include "softlight/SL_KeySym.hpp"
#include "softlight/SL_WindowBufferCocoa.hpp"
#include "softlight/SL_RenderWindowCocoa.hpp"
#include "softlight/SL_WindowEvent.hpp"



/*-----------------------------------------------------------------------------
    Application Initialization
-----------------------------------------------------------------------------*/
int sl_window_init_app()
{
    static int amInited = 0;
    static pthread_mutex_t mtxInit = PTHREAD_MUTEX_INITIALIZER;

    pthread_mutex_lock(&mtxInit);
    if (!amInited)
    {
        [NSApplication sharedApplication];
        [NSApp finishLaunching];
        amInited = true;
    }
    pthread_mutex_unlock(&mtxInit);

    return amInited;
}



/*-----------------------------------------------------------------------------
    Window Object
-----------------------------------------------------------------------------*/
@interface SL_CocoaWindow: NSWindow
{
    NSAutoreleasePool* mPool;
    NSTrackingArea* mTracking;
}
@end



@implementation SL_CocoaWindow

- (id)initWithContentRect:(NSRect)contentRect styleMask:(NSWindowStyleMask)style backing:(NSBackingStoreType)backingStoreType defer:(BOOL)flag
{
    if (self = [super initWithContentRect:contentRect styleMask:style backing:backingStoreType defer:flag])
    {
        mPool = [[NSAutoreleasePool alloc] init];
        mTracking = 0;
        [self updateTrackingRect];
    }

    return self;
}

-(void)updateTrackingRect
{
    NSView* view = [self contentView];
    if (!view)
    {
        return;
    }

    if (mTracking)
    {
        [view removeTrackingArea:mTracking];
        mTracking = nil;
    }

    NSTrackingAreaOptions options = (NSTrackingAreaOptions)NSTrackingMouseEnteredAndExited|NSTrackingInVisibleRect|NSTrackingActiveAlways|NSTrackingMouseMoved;
    NSTrackingArea* area = [[[NSTrackingArea alloc] initWithRect:[view frame] options:options owner:self userInfo:nil] autorelease];
    [view addTrackingArea:area];
}

-(void)dealloc
{
    [super dealloc];
}

-(void)drain
{
    [mPool release];
    mPool = [[NSAutoreleasePool alloc] init];
}

-(BOOL)canBecomeKeyWindow
{
    return YES;
}

- (void)keyDown:(NSEvent *)event
{
    //[super keyDown:event];
    (void)event;
}

@end



/*-----------------------------------------------------------------------------
    Window Delegate
-----------------------------------------------------------------------------*/
@interface SL_CocoaWinDelegate: NSObject<NSWindowDelegate>
{
    NSWindow* pWindow;
}
@end

@implementation SL_CocoaWinDelegate
- (instancetype)initWithWindow: (NSWindow*)window
{
    if (self = [super init])
    {
        pWindow = window;
    }

    return self;
}

- (void)dealloc
{
    [super dealloc];
}

- (void)windowWillClose: (NSNotification*)notification
{
    (void)notification;
    NSEvent* closeEvent = [NSEvent otherEventWithType:NSEventTypeApplicationDefined location:CGPointMake(0.0, 0.0) modifierFlags:(NSEventModifierFlags)0 timestamp:0.0 windowNumber:[pWindow windowNumber] context:nil subtype:SL_WinEventType::WIN_EVENT_CLOSING data1:0 data2:0];
    [pWindow postEvent:closeEvent atStart:NO];
}

- (void)windowDidResize:(NSNotification*)notification
{
    [(SL_CocoaWindow*)pWindow updateTrackingRect];

    (void)notification;
    NSRect    frame     = [pWindow frame];
    NSInteger w         = (NSInteger)frame.size.width;
    NSInteger h         = (NSInteger)frame.size.height;
    NSEvent*  resizeEvt = [NSEvent otherEventWithType:NSEventTypeApplicationDefined location:CGPointMake(0.0, 0.0) modifierFlags:(NSEventModifierFlags)0 timestamp:0.0 windowNumber:[pWindow windowNumber] context:nil subtype:SL_WinEventType::WIN_EVENT_RESIZED data1:w data2:h];
    [pWindow postEvent:resizeEvt atStart:NO];
}

- (void)windowDidMove:(NSNotification *)notification
{
    (void)notification;
    NSInteger x       = (NSInteger)pWindow.frame.origin.x;
    NSInteger y       = (NSInteger)pWindow.frame.origin.y;
    NSEvent*  moveEvt = [NSEvent otherEventWithType:NSEventTypeApplicationDefined location:CGPointMake(0.0, 0.0) modifierFlags:(NSEventModifierFlags)0 timestamp:0.0 windowNumber:[pWindow windowNumber] context:nil subtype:SL_WinEventType::WIN_EVENT_MOVED data1:x data2:y];
    [pWindow postEvent:moveEvt atStart:NO];
}

- (void)windowDidExpose:(NSNotification *)notification
{
    (void)notification;
    NSEvent* exposeEvt = [NSEvent otherEventWithType:NSEventTypeApplicationDefined location:CGPointMake(0.0, 0.0) modifierFlags:(NSEventModifierFlags)0 timestamp:0.0 windowNumber:[pWindow windowNumber] context:nil subtype:SL_WinEventType::WIN_EVENT_EXPOSED data1:0 data2:0];
    [pWindow postEvent:exposeEvt atStart:NO];
}

- (void)windowDidChangeOcclusionState:(NSNotification *)notification
{
    (void)notification;
    NSInteger       amVisible = NSWindowOcclusionStateVisible == [pWindow occlusionState];
    SL_WinEventType srEvtType = amVisible ? SL_WinEventType::WIN_EVENT_EXPOSED : SL_WinEventType::WIN_EVENT_HIDDEN;
    NSEvent*        hideEvt   = [NSEvent otherEventWithType:NSEventTypeApplicationDefined location:CGPointMake(0.0, 0.0) modifierFlags:(NSEventModifierFlags)0 timestamp:0.0 windowNumber:[pWindow windowNumber] context:nil subtype:srEvtType data1:amVisible data2:0];
    [pWindow postEvent:hideEvt atStart:NO];
}

- (void)windowDidMiniaturize:(NSNotification*)notification
{
    (void)notification;
    NSEvent* hideEvt = [NSEvent otherEventWithType:NSEventTypeApplicationDefined location:CGPointMake(0.0, 0.0) modifierFlags:(NSEventModifierFlags)0 timestamp:0.0 windowNumber:[pWindow windowNumber] context:nil subtype:SL_WinEventType::WIN_EVENT_HIDDEN data1:0 data2:0];
    [pWindow postEvent:hideEvt atStart:NO];
}

- (void)windowDidDeminiaturize:(NSNotification *)notification
{
    (void)notification;
    NSEvent* showEvt = [NSEvent otherEventWithType:NSEventTypeApplicationDefined location:CGPointMake(0.0, 0.0) modifierFlags:(NSEventModifierFlags)0 timestamp:0.0 windowNumber:[pWindow windowNumber] context:nil subtype:SL_WinEventType::WIN_EVENT_EXPOSED data1:0 data2:0];
    [pWindow postEvent:showEvt atStart:NO];
}

@end



/*-----------------------------------------------------------------------------
* SL_RenderWindowCocoa Class
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
SL_RenderWindowCocoa::~SL_RenderWindowCocoa() noexcept
{
    if (this->valid() && destroy() != 0)
    {
        LS_LOG_ERR("Unable to properly close the render window ", this, " during destruction.");
    }
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
SL_RenderWindowCocoa::SL_RenderWindowCocoa() noexcept :
    SL_RenderWindow{},
    mWindow{nullptr},
    mDelegate{nullptr},
    mLastEvent{nullptr},
    mKeysRepeat{true},
    mCaptureMouse{false},
    mMouseX{0},
    mMouseY{0}
{}


/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
SL_RenderWindowCocoa::SL_RenderWindowCocoa(const SL_RenderWindowCocoa& rw) noexcept :
    SL_RenderWindowCocoa{} // delegate
{
    // delegate some more
    *this = rw;
}


/*-------------------------------------
 * Move Constructor
-------------------------------------*/
SL_RenderWindowCocoa::SL_RenderWindowCocoa(SL_RenderWindowCocoa&& rw) noexcept :
    SL_RenderWindow{std::move(rw)},
    mWindow{rw.mWindow},
    mDelegate{rw.mDelegate},
    mLastEvent{rw.mLastEvent},
    mKeysRepeat{rw.mKeysRepeat},
    mCaptureMouse{rw.mCaptureMouse},
    mMouseX{rw.mMouseX},
    mMouseY{rw.mMouseY}
{
    rw.mWindow = nullptr;
    rw.mDelegate = nullptr;
    rw.mLastEvent = nullptr;
    rw.mKeysRepeat = true;
    rw.mCaptureMouse = false;
    rw.mMouseX = 0;
    rw.mMouseY = 0;
}


/*-------------------------------------
 * Copy Operator
-------------------------------------*/
SL_RenderWindowCocoa& SL_RenderWindowCocoa::operator=(const SL_RenderWindowCocoa& rw) noexcept
{
    if (this == &rw)
    {
        return *this;
    }

    this->destroy();

    SL_RenderWindowCocoa* const pWindow = static_cast<SL_RenderWindowCocoa*>(rw.clone());

    if (pWindow && pWindow->valid())
    {
        // handle the base class
        SL_RenderWindow::operator=(rw);
        *this = std::move(*pWindow);
    }

    delete pWindow;

    return *this;
}


/*-------------------------------------
 * Move Operator
-------------------------------------*/
SL_RenderWindowCocoa& SL_RenderWindowCocoa::operator=(SL_RenderWindowCocoa&& rw) noexcept
{
    if (this == &rw)
    {
        return *this;
    }

    if (this->valid())
    {
        this->destroy();
    }

    // handle the base class
    SL_RenderWindow::operator=(std::move(rw));
    mWindow = rw.mWindow;
    rw.mWindow = nullptr;

    mDelegate = rw.mDelegate;
    rw.mDelegate = nullptr;

    mLastEvent = rw.mLastEvent;
    rw.mLastEvent = nullptr;

    mKeysRepeat = rw.mKeysRepeat;
    rw.mKeysRepeat = true;

    mCaptureMouse = rw.mCaptureMouse;
    rw.mCaptureMouse = false;

    mMouseX = rw.mMouseX;
    rw.mMouseX = 0;

    mMouseY = rw.mMouseY;
    rw.mMouseY = 0;

    return *this;
}



/*-------------------------------------
 * Set the window title
-------------------------------------*/
int SL_RenderWindowCocoa::set_title(const char* const pName) noexcept
{
    if (!valid())
    {
        return -1;
    }

    NSWindow* win = (NSWindow*)mWindow;
    [win setTitle:[NSString stringWithUTF8String:pName]];

    return 0;
}



/*-------------------------------------
 * Create a window
-------------------------------------*/
int SL_RenderWindowCocoa::init(unsigned width, unsigned height) noexcept
{
    assert(!this->valid());

    LS_LOG_MSG("SL_RenderWindowCocoa ", this, " initializing");
    sl_window_init_app();

    NSUInteger windowStyle = NSWindowStyleMaskTitled  | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskFullSizeContentView;
    NSRect     windowRect  = NSMakeRect(0.0, 0.0, width, height);
    SL_CocoaWindow* window  = [[SL_CocoaWindow alloc] initWithContentRect:windowRect styleMask:(NSWindowStyleMask)windowStyle backing:(NSBackingStoreType)1 defer:NO];

    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

    NSMenu* menubar = [[[NSMenu alloc] init] autorelease];
    NSMenuItem* appMenuItem = [[[NSMenuItem alloc] init] autorelease];
    [menubar addItem:appMenuItem];
    [NSApp setMainMenu:menubar];

    // Add quit to the app menu
    NSMenu* appMenu = [[[NSMenu alloc] init] autorelease];
    NSString* appName = [[NSProcessInfo processInfo] processName];
    NSString* quitTitle = [@"Quit " stringByAppendingString:appName];
    NSMenuItem* quitMenuItem = [[[NSMenuItem alloc] initWithTitle:quitTitle action:@selector(terminate:) keyEquivalent:@"q"] autorelease];
    [appMenu addItem:quitMenuItem];
    [appMenuItem setSubmenu:appMenu];

    SL_CocoaWinDelegate* delegate = [[SL_CocoaWinDelegate alloc] initWithWindow:window];
    [window setDelegate:delegate];
    [window.contentView setAutoresizesSubviews:TRUE];
    [window setTitle:@"SoftLight"];
    [window setBackgroundColor:NSColor.blackColor];
    [window setAcceptsMouseMovedEvents:YES];
    [window center];
    [window makeKeyAndOrderFront:nil];

    mCurrentState = WindowStateInfo::WINDOW_STARTED;
    mWindow = (void*)window;
    mDelegate = (void*)delegate;

    LS_LOG_MSG(
        "Done. Successfully initialized SL_RenderWindowCocoa ", this, '.',
        "\n\tDisplay:    ", mWindow,
        "\n\tWindow ID:  ", mDelegate,
        "\n\tResolution: ", width, 'x', height,
        "\n\tDPI/Scale:  ", this->dpi(),
        "\n\tPosition:   ", 0,  'x', 0);

    return 0;
}



/*-------------------------------------
 * Destroy the current window and release all resources
-------------------------------------*/
int SL_RenderWindowCocoa::destroy() noexcept
{
    if (valid())
    {
        if (mWindow)
        {
            NSWindow* pWindow = (NSWindow*)mWindow;
            SL_CocoaWinDelegate* pDelegate = (SL_CocoaWinDelegate*)mDelegate;

            [pWindow performClose:pDelegate];
            mWindow = nullptr;

            [pDelegate release];
            mDelegate = nullptr;
        }

        if (mLastEvent)
        {
            NSEvent* evt = (NSEvent*)mLastEvent;
            [NSApp sendEvent:evt];
            [evt release];
            //[(NSEvent*)mLastEvent release];
            mLastEvent = nullptr;
        }

        mKeysRepeat = true;
        mCaptureMouse = false;
        mMouseX = 0;
        mMouseY = 0;

    }

    mCurrentState = WindowStateInfo::WINDOW_CLOSED;

    return 0;
}



/*-------------------------------------
* Retrieve the window width
-------------------------------------*/
unsigned SL_RenderWindowCocoa::width() const noexcept
{
    if (!valid())
    {
        return 0;
    }

    NSWindow* pWindow = (NSWindow*)mWindow;
    return (unsigned)pWindow.frame.size.width;
}



/*-------------------------------------
* Retrieve the window height
-------------------------------------*/
unsigned SL_RenderWindowCocoa::height() const noexcept
{
    if (!valid())
    {
        return 0;
    }

    NSWindow* pWindow = (NSWindow*)mWindow;
    return (unsigned)pWindow.frame.size.height;
}



/*-------------------------------------
* Retrieve the window size
-------------------------------------*/
void SL_RenderWindowCocoa::get_size(unsigned& w, unsigned& h) const noexcept
{
    if (valid())
    {
        NSWindow* pWindow = (NSWindow*)mWindow;
        w = (unsigned)pWindow.frame.size.height;
        h = (unsigned)pWindow.frame.size.height;
    }
}



/*-------------------------------------
* Set the window size
-------------------------------------*/
bool SL_RenderWindowCocoa::set_size(unsigned w, unsigned h) noexcept
{
    if (!valid() || !w || !h)
    {
        return false;
    }

    NSWindow* pWindow = (NSWindow*)mWindow;
    NSRect frame = [pWindow frame];
    if ((unsigned)frame.size.width == w && (unsigned)(frame.size.height) == h)
    {
        LS_LOG_ERR("Window size unchanged.");
        return true;
    }

    frame.size = CGSizeMake((CGFloat)w, (CGFloat)h);
    [pWindow setFrame:frame display:YES animate:YES];

    NSEvent* resizeEvt = [NSEvent otherEventWithType:NSEventTypeApplicationDefined location:CGPointMake(0.0, 0.0) modifierFlags:(NSEventModifierFlags)0 timestamp:0.0 windowNumber:[pWindow windowNumber] context:nil subtype:SL_WinEventType::WIN_EVENT_RESIZED data1:(NSInteger)w data2:(NSInteger)h];
    [pWindow postEvent:resizeEvt atStart:NO];

    return true;
}



/*-------------------------------------
* Get the window position (X)
-------------------------------------*/
int SL_RenderWindowCocoa::x_position() const noexcept
{
    if (!valid())
    {
        return 0;
    }

    NSWindow* pWindow = (NSWindow*)mWindow;
    return (unsigned)pWindow.frame.origin.x;
}



/*-------------------------------------
* Get the window position (Y)
-------------------------------------*/
int SL_RenderWindowCocoa::y_position() const noexcept
{
    if (!valid())
    {
        return 0;
    }

    NSWindow* pWindow = (NSWindow*)mWindow;
    return (unsigned)pWindow.frame.origin.y;
}



/*-------------------------------------
* Get the window position
-------------------------------------*/
bool SL_RenderWindowCocoa::get_position(int& x, int& y) const noexcept
{
    if (!valid())
    {
        return false;
    }

    NSWindow* pWindow = (NSWindow*)mWindow;
    x = (int)pWindow.frame.origin.x;
    y = (int)pWindow.frame.origin.y;

    return true;
}



/*-------------------------------------
* Set the window size
-------------------------------------*/
bool SL_RenderWindowCocoa::set_position(int x, int y) noexcept
{
    if (!valid())
    {
        return false;
    }

    NSWindow* pWindow = (NSWindow*)mWindow;
    NSRect frame = [pWindow frame];
    frame.origin = CGPointMake((CGFloat)x, (CGFloat)y);
    [pWindow setFrame:frame display:YES animate:YES];

    NSEvent* moveEvt = [NSEvent otherEventWithType:NSEventTypeApplicationDefined location:CGPointMake(0.0, 0.0) modifierFlags:(NSEventModifierFlags)0 timestamp:0.0 windowNumber:[pWindow windowNumber] context:nil subtype:SL_WinEventType::WIN_EVENT_MOVED data1:(NSInteger)x data2:(NSInteger)y];
    [pWindow postEvent:moveEvt atStart:NO];

    return true;
}



/*-------------------------------------
 * Duplicate a window
-------------------------------------*/
SL_RenderWindow* SL_RenderWindowCocoa::clone() const noexcept
{
    const SL_RenderWindowCocoa& self = *this; // nullptr check
    SL_RenderWindowCocoa* pWindow = new(std::nothrow) SL_RenderWindowCocoa(self);

    return pWindow;
}



/*-------------------------------------
 * Update the window state
-------------------------------------*/
void SL_RenderWindowCocoa::update() noexcept
{
    // sanity check
    if (!valid())
    {
        return;
    }

    SL_CocoaWindow* pWindow = (SL_CocoaWindow*)mWindow;
    [pWindow drain];

    NSEvent* pEvent = nil;

    if (mLastEvent)
    {
        NSEvent* evt = (NSEvent*)mLastEvent;
        [NSApp sendEvent:evt];
        mLastEvent = nullptr;
    }

    switch (mCurrentState)
    {
        // The window was starting to close in the last frame. Destroy it now
        case WindowStateInfo::WINDOW_CLOSING:
            destroy();
            return;

        case WindowStateInfo::WINDOW_STARTED:
            // fall-through
            run();

        case WindowStateInfo::WINDOW_RUNNING:
            // Perform a non-blocking poll if we're not paused
            pEvent = [pWindow nextEventMatchingMask:NSEventMaskAny untilDate:nil inMode:NSDefaultRunLoopMode dequeue:YES];
            break;

        case WindowStateInfo::WINDOW_PAUSED:
            // Perform a blocking event check while the window is paused.
            pEvent = [pWindow nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate distantFuture] inMode:NSDefaultRunLoopMode dequeue:YES];
            break;

        default:
            // We should not be in a "starting" or "closed" state
            LS_LOG_ERR("Encountered unexpected window state ", mCurrentState, '.');
            assert(false); // assertions are disabled on release builds
            mCurrentState = WindowStateInfo::WINDOW_CLOSING;
            return;
    }

    if (!pEvent)
    {
        return;
    }

    if (pEvent.type == NSEventTypeMouseMoved && mCaptureMouse)
    {
        CGRect viewFrame = [pWindow convertRectToScreen:[pWindow.contentView frame]];
        CGRect screenFrame = [[[NSScreen screens] firstObject] frame];
        CGFloat px = viewFrame.origin.x + (viewFrame.size.width * 0.5f);
        CGFloat py = screenFrame.size.height - (viewFrame.origin.y + (viewFrame.size.height * 0.5f));
        CGPoint p = NSMakeRect(px, py, 0.f, 0.f).origin;

        CGWarpMouseCursorPosition(p);
    }

    mLastEvent = pEvent;
}



/*-------------------------------------
-------------------------------------*/
bool SL_RenderWindowCocoa::pause() noexcept
{
    // state should only be changed for running windows
    // Otherwise, the window is either starting or stopping
    if (!valid())
    {
        return false;
    }

    // Use a switch statement so the compiler can provide a warning in case
    // a state isn't being properly handled.
    switch (mCurrentState)
    {
        // Only these cases can be used to go into a paused state
        case WindowStateInfo::WINDOW_STARTED:
            // TODO: ?
            LS_LOG_MSG("Window started in a paused state.");
        case WindowStateInfo::WINDOW_RUNNING:
        case WindowStateInfo::WINDOW_PAUSED:
        case WindowStateInfo::WINDOW_CLOSING:
            mCurrentState = WindowStateInfo::WINDOW_PAUSED;
            break;

        // These states can't be used to transition to a paused state
        case WindowStateInfo::WINDOW_CLOSED:
        case WindowStateInfo::WINDOW_STARTING:
            assert(false); // fail in case of error
            break;
    }

    return mCurrentState == WindowStateInfo::WINDOW_PAUSED;
}



/*-------------------------------------
-------------------------------------*/
bool SL_RenderWindowCocoa::run() noexcept
{
    // state should only be changed for running windows
    // Otherwise, the window is either starting or stopping
    if (!valid())
    {
        return false;
    }

    // Use a switch statement so the compiler can provide a warning in case
    // a state isn't being properly handled.
    switch (mCurrentState)
    {
        // Only these cases can be used to go into a paused state
        case WindowStateInfo::WINDOW_STARTED:
            // TODO: ?
            LS_LOG_MSG("Window started in a running state.");
        case WindowStateInfo::WINDOW_CLOSING:
        case WindowStateInfo::WINDOW_RUNNING:
        case WindowStateInfo::WINDOW_PAUSED:
            mCurrentState = WindowStateInfo::WINDOW_RUNNING;
        break;

        // These states can't be used to transition to a paused state
        case WindowStateInfo::WINDOW_CLOSED:
        case WindowStateInfo::WINDOW_STARTING:
            assert(false); // fail in case of error
            break;
    }

    return mCurrentState == WindowStateInfo::WINDOW_RUNNING;
}



/*-------------------------------------
-------------------------------------*/
bool SL_RenderWindowCocoa::has_event() const noexcept
{
    return mLastEvent != nullptr;
}



/*-------------------------------------
-------------------------------------*/
bool SL_RenderWindowCocoa::peek_event(SL_WindowEvent* const pEvent) noexcept
{
    memset(pEvent, '\0', sizeof(SL_WindowEvent));
    if (!has_event())
    {
        return false;
    }

    NSEvent* evt = (NSEvent*)mLastEvent;
    NSEventType type = [evt type];
    NSEventModifierFlags modFlags = (NSEventModifierFlags)0;
    NSPoint mousePos;
    int winX, winY;
    unsigned winW, winH;

    pEvent->pNativeWindow = (ptrdiff_t)[evt window];

    if (type == NSEventTypeApplicationDefined)
    {
        switch ((SL_WinEventType)[evt subtype])
        {
            case SL_WinEventType::WIN_EVENT_CLOSING:
                mCurrentState = WindowStateInfo::WINDOW_CLOSING;
                pEvent->type = SL_WinEventType::WIN_EVENT_CLOSING;
                return true;

            case SL_WinEventType::WIN_EVENT_RESIZED:
                pEvent->type = SL_WinEventType::WIN_EVENT_RESIZED;
                get_position(winX, winY);
                pEvent->window.x = (int16_t)winX;
                pEvent->window.y = (int16_t)winY;
                pEvent->window.width = (uint16_t)[evt data1];
                pEvent->window.height = (uint16_t)[evt data2];
                return true;

            case SL_WinEventType::WIN_EVENT_MOVED:
                pEvent->type = SL_WinEventType::WIN_EVENT_MOVED;
                get_position(winX, winY);
                get_size(winW, winH);
                pEvent->window.x = (int16_t)winX;
                pEvent->window.y = (int16_t)winY;
                pEvent->window.width = (uint16_t)winW;
                pEvent->window.height = (uint16_t)winH;
                return true;

            case SL_WinEventType::WIN_EVENT_EXPOSED:
                pEvent->type = SL_WinEventType::WIN_EVENT_EXPOSED;
                return true;

            case SL_WinEventType::WIN_EVENT_HIDDEN:
                pEvent->type = SL_WinEventType::WIN_EVENT_HIDDEN;
                return true;

            default:
                LS_DEBUG_ASSERT(false); // did I miss one?
                return pEvent->type = SL_WinEventType::WIN_EVENT_UNKNOWN;
                return false;
        }
    }

    switch (type)
    {
        case NSEventTypeKeyDown:
            modFlags = [evt modifierFlags];
            pEvent->type = WIN_EVENT_KEY_DOWN;
            pEvent->keyboard.keysym = (SL_KeySymbol)[evt keyCode];
            pEvent->keyboard.key = (uint8_t)(mKeysRepeat ? 0 : [[evt charactersIgnoringModifiers] characterAtIndex:0]);
            pEvent->keyboard.capsLock = (uint8_t)(0 != (modFlags & NSEventModifierFlagCapsLock));
            pEvent->keyboard.numLock = (uint8_t)(0 != (modFlags & NSEventModifierFlagNumericPad));
            pEvent->keyboard.scrollLock = (uint8_t)(0 != (modFlags & NSEventModifierFlagFunction));
            break;

        case NSEventTypeKeyUp:
            modFlags = [evt modifierFlags];
            pEvent->type = WIN_EVENT_KEY_UP;
            pEvent->keyboard.keysym = (SL_KeySymbol)[evt keyCode];
            pEvent->keyboard.key = (uint8_t)(mKeysRepeat ? 0 : [[evt charactersIgnoringModifiers] characterAtIndex:0]);
            pEvent->keyboard.capsLock = (uint8_t)(0 != (modFlags & NSEventModifierFlagCapsLock));
            pEvent->keyboard.numLock = (uint8_t)(0 != (modFlags & NSEventModifierFlagNumericPad));
            pEvent->keyboard.scrollLock = (uint8_t)(0 != (modFlags & NSEventModifierFlagFunction));
            break;

        case NSEventTypeMouseEntered:
            pEvent->type = SL_WinEventType::WIN_EVENT_MOUSE_ENTER;
            mousePos = [evt locationInWindow];
            pEvent->mousePos.x = (int16_t)mousePos.x;
            pEvent->mousePos.y = (int16_t)mousePos.y;
            break;

        case NSEventTypeMouseExited:
            pEvent->type = SL_WinEventType::WIN_EVENT_MOUSE_LEAVE;
            mousePos = [evt locationInWindow];
            pEvent->mousePos.x = (int16_t)mousePos.x;
            pEvent->mousePos.y = (int16_t)mousePos.y;
            break;

        case NSEventTypeLeftMouseDown:
            mousePos = [evt locationInWindow];
            pEvent->type = WIN_EVENT_MOUSE_BUTTON_DOWN;
            pEvent->mouseButton.mouseButton1 = 1;
            pEvent->mouseButton.x = (int16_t)mousePos.x;
            pEvent->mouseButton.y = (int16_t)mousePos.y;
            break;

        case NSEventTypeRightMouseDown:
            mousePos = [evt locationInWindow];
            pEvent->type = WIN_EVENT_MOUSE_BUTTON_DOWN;
            pEvent->mouseButton.mouseButton2 = 1;
            pEvent->mouseButton.x = (int16_t)mousePos.x;
            pEvent->mouseButton.y = (int16_t)mousePos.y;
            break;

        case NSEventTypeOtherMouseDown:
            mousePos = [evt locationInWindow];
            pEvent->type = WIN_EVENT_MOUSE_BUTTON_DOWN;
            if (3 == [evt buttonNumber])
            {
                pEvent->mouseButton.mouseButton3 = 1;
            }
            else
            {
                pEvent->mouseButton.mouseButtonN = 1;
            }
            pEvent->mouseButton.x = (int16_t)mousePos.x;
            pEvent->mouseButton.y = (int16_t)mousePos.y;
            break;

        case NSEventTypeLeftMouseUp:
            mousePos = [evt locationInWindow];
            pEvent->type = WIN_EVENT_MOUSE_BUTTON_UP;
            pEvent->mouseButton.mouseButton1 = 1;
            pEvent->mouseButton.x = (int16_t)mousePos.x;
            pEvent->mouseButton.y = (int16_t)mousePos.y;
            break;

        case NSEventTypeRightMouseUp:
            mousePos = [evt locationInWindow];
            pEvent->type = WIN_EVENT_MOUSE_BUTTON_UP;
            pEvent->mouseButton.mouseButton2 = 1;
            pEvent->mouseButton.x = (int16_t)mousePos.x;
            pEvent->mouseButton.y = (int16_t)mousePos.y;
            break;

        case NSEventTypeOtherMouseUp:
            mousePos = [evt locationInWindow];
            pEvent->type = WIN_EVENT_MOUSE_BUTTON_UP;
            if (3 == [evt buttonNumber])
            {
                pEvent->mouseButton.mouseButton3 = 1;
            }
            else
            {
                pEvent->mouseButton.mouseButtonN = 1;
            }
            pEvent->mouseButton.x = (int16_t)mousePos.x;
            pEvent->mouseButton.y = (int16_t)mousePos.y;
            break;

        case NSEventTypeScrollWheel:
            pEvent->type = SL_WinEventType::WIN_EVENT_MOUSE_WHEEL_MOVED;
            pEvent->wheel.x = (int16_t)[evt deltaX];
            pEvent->wheel.y = (int16_t)[evt deltaY];
            pEvent->wheel.up = (4 == [evt buttonNumber]);
            pEvent->wheel.down = (5 == [evt buttonNumber]);
            break;

        case NSEventTypeMouseMoved:
            pEvent->type = WIN_EVENT_MOUSE_MOVED;
            mousePos = [evt locationInWindow];
            pEvent->mousePos.x = (int16_t)mousePos.x;
            pEvent->mousePos.y = (int16_t)mousePos.y;
            pEvent->mousePos.dx = -(int16_t)[evt deltaX];
            pEvent->mousePos.dy = -(int16_t)[evt deltaY];
            mMouseX = pEvent->mousePos.x;
            mMouseY = pEvent->mousePos.y;
            break;

        default: // unhandled event
            pEvent->type = SL_WinEventType::WIN_EVENT_UNKNOWN;
            return false;
    }

    return true;
}


/*-------------------------------------
* Remove an event from the event queue
-------------------------------------*/
bool SL_RenderWindowCocoa::pop_event(SL_WindowEvent* const pEvent) noexcept
{
    bool ret = peek_event(pEvent);
    [NSApp sendEvent:(NSEvent*)mLastEvent];
    mLastEvent = nullptr;
    return ret;
}


/*-------------------------------------
 * Enable or disable repeating keys
-------------------------------------*/
bool SL_RenderWindowCocoa::set_keys_repeat(bool doKeysRepeat) noexcept
{
    mKeysRepeat = doKeysRepeat;
    return mKeysRepeat;
}


/*-------------------------------------
 * Render a framebuffer to the current window
-------------------------------------*/
void SL_RenderWindowCocoa::render(SL_WindowBuffer& buffer) noexcept
{
    assert(this->valid());
    assert(buffer.native_handle() != nullptr);

    SL_WindowBufferCocoa& buf  = static_cast<SL_WindowBufferCocoa&>(buffer);
    //CGImageRef            img  = (CGImageRef)buf.native_handle();
    NSWindow*             win  = (NSWindow*)mWindow;

    unsigned bpp = (unsigned)buf.texture().bpp();
    CGImageRef img = CGImageCreate(
        buf.width(),
        buf.height(),
        CHAR_BIT,
        CHAR_BIT*bpp,
        bpp*buf.width(),
        (CGColorSpaceRef)buf.mColorSpace,
        kCGImageAlphaNoneSkipFirst|kCGBitmapByteOrder32Host,
        (CGDataProviderRef)buf.mImageProvider,
        nullptr,
        NO,
        kCGRenderingIntentDefault);

    [[[win contentView] layer] setContents:(__bridge id)img];
    [[[win contentView] layer] display];

    CGImageRelease(img);
}


/*-------------------------------------
 * Check if the mouse is captured
-------------------------------------*/
void SL_RenderWindowCocoa::set_mouse_capture(bool isCaptured) noexcept
{
    if (!valid())
    {
        return;
    }

    mCaptureMouse = isCaptured;

    if (mCaptureMouse)
    {
        NSWindow* pWindow = (NSWindow*)mWindow;
        CGRect screenFrame = [[[NSScreen screens] firstObject] frame];
        CGFloat x = pWindow.frame.origin.x;
        CGFloat y = pWindow.frame.origin.y;
        CGFloat px = x + pWindow.frame.size.width * (CGFloat)0.5;
        CGFloat py = screenFrame.size.height - (y + pWindow.frame.size.height * (CGFloat)0.5);

        CGWarpMouseCursorPosition(CGPointMake(px, py));
    }
}


/*-------------------------------------
 * Check if the mouse is captured
-------------------------------------*/
bool SL_RenderWindowCocoa::is_mouse_captured() const noexcept
{
    return mCaptureMouse;
}


/*-------------------------------------
 * Check if the mouse is captured
-------------------------------------*/
unsigned SL_RenderWindowCocoa::dpi() const noexcept
{
    if (!valid())
    {
        return 0;
    }

    NSWindow* pWindow = (NSWindow*)mWindow;
    CGDirectDisplayID displayId = 0;
    uint32_t displaysFound = 0;

    if (kCGErrorSuccess != CGGetDisplaysWithRect(pWindow.frame, 1, &displayId, &displaysFound) || !displaysFound)
    {
        return 0;
    }

    CGSize displayMMs = CGDisplayScreenSize(displayId);
    return (unsigned)(displayMMs.width / (CGFloat)25.4);
}
