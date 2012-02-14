  /*
 int currButtonVal = 0;
 int prevButtonVal = 0;
 int _currButton = 0;
 
 int getButton(){
   _currButton = digitalRead(buttonPin);
   //currButtonVal = digitalRead(buttonPin);  //read our button
    
    if(currButtonVal != prevButtonVal){ //check for state change
      prevButtonVal = currButtonVal;  //set prev to curr
      
      if(prevButtonVal == 1){ 
        _currButton ++ ;           //add one on each press 
                                  //(this is for if a counter is desired in the future)
        
        if(_currButton > 1){       //count between 0 and 1.
          _currButton = 0;
        }
      }
    }
    //setButtonLed(currButton)
    
    return _currButton;
 }*/
