#include "HUSKYLENS.h"
#include "Servo.h"

HUSKYLENS huskylens;

Servo rotation; // rotation (0 right, 180 left)
Servo servo1;// arm up1
Servo servo2;//arm up2 (180 forward,)
Servo claw;// claw (0 closed, 90 open)

const int ID1 = 1;  // First learned results. Colored result on HuskyLens screen
const int ID2 = 2;  // Second learned results. Colored result on HuskyLens screen
const int ID3 = 3;  // Third learned results. Colored result on HuskyLens screen

struct Item {
  String name;
  int quantity;
  int id; // Added item ID
};

const int maxItems = 4;  
Item items[maxItems] = {{"apple", 4, 1}, {"pear", 5, 2}, {"water", 6, 3}, {"mug", 10, 4}};

struct Order {
  String itemName;
  int quantity;
};

const int maxOrders = 10; 
Order orders[maxOrders];
int currentOrderIndex = 0;

String prod_name = "";
int prod_quantity = 0;

void moveServos(int rotationAngle, int servo1Angle, int servo2Angle, int clawAngle) {
  rotation.write(rotationAngle); //rotationAngle: Angle for rotation servo.
  delay(1000);
  servo1.write(servo1Angle); //servo1Angle: Angle for servo1
  delay(1000);
  servo2.write(servo2Angle); //servo2Angle: Angle for servo2.
  delay(1000);
  claw.write(clawAngle); //clawAngle: Angle for the claw servo.
}

void resetServos() {
  moveServos(180, 30, 90, 90);
}

void moveItemWithID1(int quantity){
  // Specific movements for item with ID 1
 
  for (int i = 0; i < quantity; ++i) {
    
    moveServos(90,180,90,20);
    delay(1000);  //
    moveServos(90,140,90,20);
    delay(1000); 
    moveServos(140,180,90,90);
    delay(1000);  
    moveServos(140,90,90,90);
    delay(1000);
    
  }
}

void moveItemWithID2(int quantity){
  // Specific movements for item with ID 2
  for (int i = 0; i < quantity; ++i) {
    
    moveServos(90,180,90,20);
    delay(1000);  //
    moveServos(90,140,90,20);
    delay(1000); 
    moveServos(180,180,90,90);
    delay(1000);  
    moveServos(180,90,90,90);
    delay(1000);
}
}

void moveItemWithID3(int quantity){
  // Specific movements for item with ID 3
  for (int i = 0; i < quantity; ++i) {
    
    moveServos(90,180,90,20);
    delay(1000); 
    moveServos(90,140,90,20);
    delay(1000); 
    moveServos(90,180,90,90);
    delay(1000);  
    moveServos(90,90,90,90);
    delay(1000);
  }
}

bool isItemLearned(int itemId) {
  // Check if the item ID is learned by the HuskyLens
  
  int learnedIDs[] = {ID1, ID2, ID3};  // Add the learned IDs
  for (int i = 0; i < sizeof(learnedIDs) / sizeof(learnedIDs[0]); ++i) {
    if (itemId == learnedIDs[i]) {
      return true;
    }
  }
  return false;
}

void printOrder(const Order& order) {
  Serial.print("Item: ");
  Serial.print(order.itemName);
  Serial.print(", Quantity: ");
  Serial.println(order.quantity);
}

void printOrders(){
  Serial.println("Current Orders:");
  for (int i = 0; i < currentOrderIndex; ++i) {
    Serial.print("Order #: ");
    Serial.print(i + 1);  // Display order number starting from 1
    Serial.print(", ");
    printOrder(orders[i]);
  }
  Serial.println("-----");
}

void prepareOrder(const String& itemName, int quantity) {
  int maxRetries = 10;  // Adjust the maximum number of retries
  int retries = 0;
  bool orderSuccessful = false; // Flag to track the order status

  while (retries < maxRetries && !orderSuccessful) {
    // Check if the detected item matches the order using HuskyLens
    if (!huskylens.request()) {
      Serial.println("Failed to request objects from Huskylens!");
      return;
    }

    HUSKYLENSResult result = huskylens.read();

    // Loop through all items to find a match
    for (int i = 0; i < maxItems; ++i) {
      // Check if the item is learned and matches the order
      if (isItemLearned(items[i].id) && result.ID == items[i].id && itemName == items[i].name) {
        // Check if the requested quantity is available in stock
        if (quantity > items[i].quantity) {
          Serial.println("Insufficient quantity in stock. Order cannot be fulfilled.");
          return;
        }

        Serial.println(itemName + " detected. Preparing order...");

        // Perform actions for preparing the order
        switch (items[i].id) {
          case ID1:
            moveItemWithID1(quantity);
         
            break;
          case ID2:
            moveItemWithID2(quantity);
    
            break;
          case ID3:
            moveItemWithID3(quantity);
           
            break;

          default:
            Serial.println("Unknown item ID");
            return;
        }

        // Update the item quantity in stock
        items[i].quantity -= quantity;
        Serial.println("Order prepared successfully.");

        // Set the flag to true to indicate a successful order
        orderSuccessful = true;

        // Exit the item loop after order preparation
        break;
      }
    }

    // If the detected item does not match the order, print a retry message
    if (!orderSuccessful) {
      Serial.println("Detected item does not match the order. Retrying...");
      delay(1000);  
      retries++;
    }
  }

  // If the order was successful, print a success message
  if (orderSuccessful) {
    Serial.println("Order placed successfully.");
  } else {
    Serial.println("Failed to detect the ordered item after multiple attempts.");
  }
}

void order_menu() {
  Serial.println("### What would you like to order? ### ");
  Serial.println("");
  // Wait for the entire order input (e.g., 'item1,quantity1,item2,quantity2,...')
  Serial.println("Enter your entire order (e.g., 'item1,quantity1,item2,quantity2,...')");

  while (Serial.available() == 0) { /* Wait for input */ }

  String orderInput = Serial.readStringUntil('\n');

  // Split the input into pairs of item and quantity
  int itemCount = 0;
  int lastCommaIndex = -1;

  for (int i = 0; i <= orderInput.length(); ++i) {
    if (i == orderInput.length() || orderInput.charAt(i) == ',') {
      String pair = orderInput.substring(lastCommaIndex + 1, i);

      // Toggle between item name and quantity
      if (itemCount % 2 == 0) {
        // Even index: item name
        prod_name = pair;
      } else {
        // Odd index: quantity
        prod_quantity = pair.toInt();

        // Validate quantity input
        if (isnan(prod_quantity) || isinf(prod_quantity) || prod_quantity < 0) {
          Serial.println("Invalid quantity format.");
          return;
        }

        // Check if the item is available and prepare the order
        prepareOrder(prod_name, prod_quantity);
        resetServos(); // Reset servos after processing each item
      }

      lastCommaIndex = i;
      itemCount++;
    }
  }

  // Check if the number of items and quantities is even
  if (itemCount % 2 != 0) {
    Serial.println("Invalid order format. Please enter both item names and quantities.");
  }

}

void setup() {
  rotation.attach(3);
  servo1.attach(5);
  servo2.attach(6);
  claw.attach(9);

  Serial.begin(9600);
  Wire.begin();
  while (!huskylens.begin(Wire)) {
    Serial.println(F("Begin failed!"));
    delay(100);
  }

  //adding the product name to the designated tag
  huskylens.setCustomName("apple", 1);
  huskylens.setCustomName("pear", 2);
  huskylens.setCustomName("water", 3);
   
  huskylens.writeAlgorithm(ALGORITHM_TAG_RECOGNITION);

  //move the arm in the default position
  resetServos();

}

void loop(){
  order_menu();
}