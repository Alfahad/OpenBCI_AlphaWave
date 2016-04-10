/*
 This is a simple iPhone App to connect to OpenBCI 32-bit Board
 By Hassan Albalawi - April 09, 2016
 */

#import <QuartzCore/QuartzCore.h>

#import "ViewController.h"
#import <AVFoundation/AVFoundation.h>

@interface ViewController ()
{
    AVAudioPlayer *_audioPlayer;
}
@end

@implementation ViewController

@synthesize rfduino;

+ (void)load
{
}

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
        UIButton *backButton = [UIButton buttonWithType:101];  // left-pointing shape
        [backButton setTitle:@"Disconnect" forState:UIControlStateNormal];
        [backButton addTarget:self action:@selector(disconnect:) forControlEvents:UIControlEventTouchUpInside];
        
        UIBarButtonItem *backItem = [[UIBarButtonItem alloc] initWithCustomView:backButton];
        [[self navigationItem] setLeftBarButtonItem:backItem];
        
        [[self navigationItem] setTitle:@"OpenBCI Alpha Monitor"];
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    // Do any additional setup after loading the view from its nib.
    
    [rfduino setDelegate:self];
//    
//    UIColor *start = [UIColor colorWithRed:58/255.0 green:108/255.0 blue:183/255.0 alpha:0.15];
//    UIColor *stop = [UIColor colorWithRed:58/255.0 green:108/255.0 blue:183/255.0 alpha:0.45];
    
    CAGradientLayer *gradient = [CAGradientLayer layer];
    gradient.frame = [self.view bounds];
//    gradient.colors = [NSArray arrayWithObjects:(id)start.CGColor, (id)stop.CGColor, nil];
    [self.view.layer insertSublayer:gradient atIndex:0];
    
    
    [self.mySwitch addTarget:self
                      action:@selector(stateChanged:) forControlEvents:UIControlEventValueChanged];
    
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (IBAction)disconnect:(id)sender
{
    NSLog(@"disconnect pressed");
    
    [rfduino disconnect];
}

- (void)sendByte:(uint8_t)byte
{
    uint8_t tx[1] = { byte };
    NSData *data = [NSData dataWithBytes:(void*)&tx length:1];
    [rfduino send:data];
}


- (void)stateChanged:(UISwitch *)switchState
{
    if ([switchState isOn]) {
        [self sendByte:'b']; // begin streaming
        self.myTextField.text = @"Streaming ...";
        [self.myTextField setTextColor:[UIColor greenColor]];


    } else {
        [self sendByte:'s']; // stop streaming
        self.myTextField.text = @"Stopped streaming!";
        [self.myTextField setTextColor:[UIColor redColor]];
    }
}


- (void)didReceive:(NSData *)data
{
    NSLog(@"RecievedRX");
    
    float Counter_Value = dataFloat(data);
    
    NSLog(@"c=%.2f", Counter_Value);
    
    NSString* string1 = [NSString stringWithFormat:@"%.2f", Counter_Value];
    
    [label1 setText:string1];
    
    if (Counter_Value < 70){
        if (Counter_Value > 30)
            {
                [self playSound];
            }
            else
            {
                [self playSound];
                [self vibrate];
            }
    }
}


- (void)vibrate{
    AudioServicesPlayAlertSound(kSystemSoundID_Vibrate);
}

- (void)playSound{
    AudioServicesPlaySystemSound (1005);
}
@end
