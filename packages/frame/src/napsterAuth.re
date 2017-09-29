let getAuthUrl apiState => {
    "https://api.napster.com/oauth/authorize?client_id=" ^ apiKey ^
        "&redirect_uri=http://" ^ hostname ^ "/&response_type=code" ^
        "&state=" ^ apiState;
