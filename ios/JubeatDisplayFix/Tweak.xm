#import <UIKit/UIKit.h>
#import <objc/runtime.h>
#import <substrate.h>

#include <math.h>

@interface EAGLView : UIView
@end

@interface GameViewController : UIViewController
- (UIView *)glView;
- (id)mainGameRenderer;
@end

@interface JDFWeakController : NSObject
@property(nonatomic, weak) GameViewController *value;
@end

@implementation JDFWeakController
@end

static char ControllerAssociationKey;

static bool IsPadGameController(GameViewController *controller)
{
    if (!controller || !MSHookIvar<bool>(controller, "isPad"))
        return false;

    id renderer = [controller mainGameRenderer];
    if (!renderer)
        return false;
    NSString *className = NSStringFromClass(object_getClass(renderer));
    return [className rangeOfString:@"Pad"].location != NSNotFound;
}

static void BindControllerToGameView(GameViewController *controller)
{
    UIView *view = [controller glView];
    if (![view isKindOfClass:[UIView class]])
        return;
    JDFWeakController *association = objc_getAssociatedObject(view, &ControllerAssociationKey);
    if (!association)
    {
        association = [JDFWeakController new];
        objc_setAssociatedObject(view, &ControllerAssociationKey, association,
                                 OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }
    association.value = controller;
}

static GameViewController *ControllerForGameView(UIView *view)
{
    JDFWeakController *association = objc_getAssociatedObject(view, &ControllerAssociationKey);
    return association.value;
}

static float DisplayScaleForView(UIView *view)
{
    CGRect bounds = CGRectStandardize(view.bounds);
    const CGFloat firstSide = fabs(bounds.size.width);
    const CGFloat secondSide = fabs(bounds.size.height);
    const CGFloat shortSide = fmin(firstSide, secondSide);
    const CGFloat longSide = fmax(firstSide, secondSide);
    if (shortSide <= 0.0 || longSide <= 0.0)
        return 0.0f;

    const double scale = fmin(shortSide / 768.0, longSide / 1024.0);
    return scale > 0.5 && scale <= 3.0 ? (float)scale : 0.0f;
}

static void ApplyDisplayScale(GameViewController *controller)
{
    if (!IsPadGameController(controller))
        return;
    UIView *view = [controller glView];
    if (!view)
        return;

    const float scale = DisplayScaleForView(view);
    if (scale == 0.0f)
        return;
    float &oldScale = MSHookIvar<float>(controller, "displayScale");
    if (fabsf(oldScale - scale) > 0.001f)
        oldScale = scale;
}

static CGPoint CanonicalTouchPoint(CGPoint point, UIView *view)
{
    CGRect bounds = CGRectStandardize(view.bounds);
    if (bounds.size.width <= bounds.size.height)
        return point;

    UIScreen *screen = view.window.screen ?: UIScreen.mainScreen;
    id<UICoordinateSpace> fixedSpace = screen.fixedCoordinateSpace;
    CGRect fixedBounds = CGRectStandardize([view convertRect:view.bounds
                                            toCoordinateSpace:fixedSpace]);
    CGPoint fixedPoint = [view convertPoint:point toCoordinateSpace:fixedSpace];
    CGPoint result = CGPointMake(fixedPoint.x - CGRectGetMinX(fixedBounds),
                                 fixedPoint.y - CGRectGetMinY(fixedBounds));
    if (!isfinite(result.x) || !isfinite(result.y) ||
        fixedBounds.size.width > fixedBounds.size.height)
        return point;
    return result;
}

%hook GameViewController

- (void)loadResources
{
    %orig;
    BindControllerToGameView(self);
    ApplyDisplayScale(self);
}

%end

%hook EAGLView

- (void)layoutSubviews
{
    %orig;
    ApplyDisplayScale(ControllerForGameView(self));
}

%end

%hook UITouch

- (CGPoint)locationInView:(UIView *)view
{
    CGPoint point = %orig;
    GameViewController *controller = ControllerForGameView(view);
    if (!IsPadGameController(controller))
        return point;
    ApplyDisplayScale(controller);
    return CanonicalTouchPoint(point, view);
}

%end
