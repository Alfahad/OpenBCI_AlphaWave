//
//  ViewController.h
//  OpenBCI-AlphaWaves
//
//  Created by Hassan Albalawi on 4/9/16.
//  2016 Hassan Albalawi. 
//

#import <UIKit/UIKit.h>

#import "RFduino.h"

@interface ViewController : UIViewController<RFduinoDelegate>
{
    __weak IBOutlet UILabel *label1;
}

@property (weak, nonatomic) IBOutlet UISwitch *mySwitch;
@property (weak, nonatomic) IBOutlet UITextField *myTextField;

@property(strong, nonatomic) RFduino *rfduino;

@end
