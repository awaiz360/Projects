import 'package:firebase_core/firebase_core.dart';
import 'package:flutter/foundation.dart';

Future initFirebase() async {
  if (kIsWeb) {
    await Firebase.initializeApp(
        options: const FirebaseOptions(
            apiKey: "AIzaSyCYFWbKLEuRtJl6ZIQodyy7NivlNprXLVA",
            authDomain: "mood-mate-hwmtub.firebaseapp.com",
            projectId: "mood-mate-hwmtub",
            storageBucket: "mood-mate-hwmtub.appspot.com",
            messagingSenderId: "398992031545",
            appId: "1:398992031545:web:da4eb27e928afbe1e132e8"));
  } else {
    await Firebase.initializeApp();
  }
}
