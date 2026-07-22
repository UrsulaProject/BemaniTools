#import <Foundation/Foundation.h>

@interface MarkerManager : NSObject
+ (NSString *)getMarkerDirectoryPath;
+ (void)setMarkerList:(NSArray<NSDictionary<NSString *, NSString *> *> *)markerList;
@end

static BOOL BMTValidateMarkerList(id object)
{
    if (![object isKindOfClass:[NSArray class]])
        return NO;

    for (id entry in (NSArray *)object)
    {
        if (![entry isKindOfClass:[NSDictionary class]])
            return NO;

        NSDictionary *dictionary = (NSDictionary *)entry;
        for (NSString *key in @[@"markerID", @"bannerName", @"version"])
        {
            if (![[dictionary objectForKey:key] isKindOfClass:[NSString class]])
                return NO;
        }
    }
    return YES;
}

static void BMTInstallMarkerList(void)
{
    NSString *markerDirectory = [MarkerManager getMarkerDirectoryPath];
    NSString *path = [markerDirectory stringByAppendingPathComponent:@"marker-list.plist"];
    NSData *data = [NSData dataWithContentsOfFile:path];
    if (!data)
        return;

    NSError *error = nil;
    id markerList = [NSPropertyListSerialization propertyListWithData:data
                                                               options:NSPropertyListImmutable
                                                                format:NULL
                                                                 error:&error];
    if (!BMTValidateMarkerList(markerList))
    {
        NSLog(@"[BemaniTools] Ignoring invalid marker list at %@: %@", path, error);
        return;
    }

    [MarkerManager setMarkerList:markerList];
    [[NSUserDefaults standardUserDefaults] synchronize];
    NSLog(@"[BemaniTools] Installed %lu marker entries from %@",
          (unsigned long)[markerList count], path);
}

%hook MarkerManager

+ (BOOL)checkRegularMarkerData
{
    // The original implementation immediately reads, validates, and saves the
    // list again, so install the plaintext list just before that pass.
    BMTInstallMarkerList();
    return %orig;
}

%end
