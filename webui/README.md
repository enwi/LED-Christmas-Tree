# LED-Christmas-Tree-WebUI

This repo contains the source code for the web user interface for https://github.com/enwi/LED-Christmas-Tree 
The build is automatic (via github actions) and the header to include in the webserver code is in the `cpp` folder.
The header contains the gziped build of the vuejs project 

## Screenshot
![chrome_2021-12-13_18-38-45](https://user-images.githubusercontent.com/17061996/145861251-56230b4b-9293-4dee-b171-dcd1be3a8d73.png)
![chrome_2021-12-13_18-39-04](https://user-images.githubusercontent.com/17061996/145861297-a6b258ae-130a-4cdd-856d-9475546481e0.png)

## Contribute
The app has a mock api of the esp when it's started in development mode (see `src/esp-api-mock.js`).
This allows for testing the ui / ux without having the tree
