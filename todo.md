- Bug: If client joins after a dynamic wall has been placed then the new client doesn't see the wall (this is because the
current place wall event is only pushed to the clients on wall creation and only on wall creation currently). May need to
add dynamic walls to the state packet as well.
