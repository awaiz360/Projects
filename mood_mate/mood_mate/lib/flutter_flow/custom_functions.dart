import 'dart:convert';
import 'dart:math' as math;

import 'package:flutter/material.dart';
import 'package:google_fonts/google_fonts.dart';
import 'package:intl/intl.dart';
import 'package:timeago/timeago.dart' as timeago;
import 'lat_lng.dart';
import 'place.dart';
import 'uploaded_file.dart';
import '/backend/backend.dart';
import 'package:cloud_firestore/cloud_firestore.dart';
import '/auth/firebase_auth/auth_util.dart';

String? newCustomFunction(
  String? tot,
  String? con,
) {
  return (((int.parse(con!) / int.parse(tot!)) * 100).round()).toString();
}

String? newCustomFunction2(
  String? n1,
  String? n2,
) {
  return (int.parse(n1!) + int.parse(n2!)).toString();
}

String? newCustomFunction3(
  String? con,
  String? tot,
) {
  return (int.parse(tot!) - int.parse(con!)).toString();
}
