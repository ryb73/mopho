open Js.Promise;
open Js.Result;

let pool = DbPool.pool;

type insertResult = {
    insertId: int
};

let create name => {
    Mysql.Queryable.query pool ("INSERT INTO users (name) VALUES ('" ^ name ^ "')")
        |> then_ (fun (result, _) => {
            switch (insertResult__from_json result) {
                | Error _ => failwith "Error converting insert result"
                | Ok { insertId } => resolve insertId
            };
        });
};

type userIdResult = { userId: int };
type userIdSelectResult = array userIdResult;

let getFromNapsterId napsterId => {
     Mysql.Queryable.query pool ("SELECT userId FROM napster_users WHERE napsterUserId = '" ^ napsterId ^ "'")
        |> then_ (fun (result, _) => {
            switch (userIdSelectResult__from_json result) {
                | Error _ => failwith "Error converting select result"
                | Ok results =>
                    switch (Js.Array.length results) {
                        | 0 => resolve None
                        | _ => resolve @@ Some results.(0).userId
                    }
            };
        });
};

let saveNapsterRefreshToken _id _refreshToken => {
    failwith "nope";
};