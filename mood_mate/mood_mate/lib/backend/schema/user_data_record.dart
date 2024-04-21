import 'dart:async';

import 'package:collection/collection.dart';

import '/backend/schema/util/firestore_util.dart';
import '/backend/schema/util/schema_util.dart';

import 'index.dart';
import '/flutter_flow/flutter_flow_util.dart';

class UserDataRecord extends FirestoreRecord {
  UserDataRecord._(
    super.reference,
    super.data,
  ) {
    _initializeFields();
  }

  // "id" field.
  int? _id;
  int get id => _id ?? 0;
  bool hasId() => _id != null;

  // "f_name" field.
  String? _fName;
  String get fName => _fName ?? '';
  bool hasFName() => _fName != null;

  // "email" field.
  String? _email;
  String get email => _email ?? '';
  bool hasEmail() => _email != null;

  // "age" field.
  int? _age;
  int get age => _age ?? 0;
  bool hasAge() => _age != null;

  // "display_name" field.
  String? _displayName;
  String get displayName => _displayName ?? '';
  bool hasDisplayName() => _displayName != null;

  // "photo_url" field.
  String? _photoUrl;
  String get photoUrl => _photoUrl ?? '';
  bool hasPhotoUrl() => _photoUrl != null;

  // "uid" field.
  String? _uid;
  String get uid => _uid ?? '';
  bool hasUid() => _uid != null;

  // "created_time" field.
  DateTime? _createdTime;
  DateTime? get createdTime => _createdTime;
  bool hasCreatedTime() => _createdTime != null;

  // "phone_number" field.
  String? _phoneNumber;
  String get phoneNumber => _phoneNumber ?? '';
  bool hasPhoneNumber() => _phoneNumber != null;

  // "q1" field.
  String? _q1;
  String get q1 => _q1 ?? '';
  bool hasQ1() => _q1 != null;

  // "q2" field.
  String? _q2;
  String get q2 => _q2 ?? '';
  bool hasQ2() => _q2 != null;

  // "q3" field.
  String? _q3;
  String get q3 => _q3 ?? '';
  bool hasQ3() => _q3 != null;

  // "q4" field.
  String? _q4;
  String get q4 => _q4 ?? '';
  bool hasQ4() => _q4 != null;

  void _initializeFields() {
    _id = castToType<int>(snapshotData['id']);
    _fName = snapshotData['f_name'] as String?;
    _email = snapshotData['email'] as String?;
    _age = castToType<int>(snapshotData['age']);
    _displayName = snapshotData['display_name'] as String?;
    _photoUrl = snapshotData['photo_url'] as String?;
    _uid = snapshotData['uid'] as String?;
    _createdTime = snapshotData['created_time'] as DateTime?;
    _phoneNumber = snapshotData['phone_number'] as String?;
    _q1 = snapshotData['q1'] as String?;
    _q2 = snapshotData['q2'] as String?;
    _q3 = snapshotData['q3'] as String?;
    _q4 = snapshotData['q4'] as String?;
  }

  static CollectionReference get collection =>
      FirebaseFirestore.instance.collection('user_data');

  static Stream<UserDataRecord> getDocument(DocumentReference ref) =>
      ref.snapshots().map((s) => UserDataRecord.fromSnapshot(s));

  static Future<UserDataRecord> getDocumentOnce(DocumentReference ref) =>
      ref.get().then((s) => UserDataRecord.fromSnapshot(s));

  static UserDataRecord fromSnapshot(DocumentSnapshot snapshot) =>
      UserDataRecord._(
        snapshot.reference,
        mapFromFirestore(snapshot.data() as Map<String, dynamic>),
      );

  static UserDataRecord getDocumentFromData(
    Map<String, dynamic> data,
    DocumentReference reference,
  ) =>
      UserDataRecord._(reference, mapFromFirestore(data));

  @override
  String toString() =>
      'UserDataRecord(reference: ${reference.path}, data: $snapshotData)';

  @override
  int get hashCode => reference.path.hashCode;

  @override
  bool operator ==(other) =>
      other is UserDataRecord &&
      reference.path.hashCode == other.reference.path.hashCode;
}

Map<String, dynamic> createUserDataRecordData({
  int? id,
  String? fName,
  String? email,
  int? age,
  String? displayName,
  String? photoUrl,
  String? uid,
  DateTime? createdTime,
  String? phoneNumber,
  String? q1,
  String? q2,
  String? q3,
  String? q4,
}) {
  final firestoreData = mapToFirestore(
    <String, dynamic>{
      'id': id,
      'f_name': fName,
      'email': email,
      'age': age,
      'display_name': displayName,
      'photo_url': photoUrl,
      'uid': uid,
      'created_time': createdTime,
      'phone_number': phoneNumber,
      'q1': q1,
      'q2': q2,
      'q3': q3,
      'q4': q4,
    }.withoutNulls,
  );

  return firestoreData;
}

class UserDataRecordDocumentEquality implements Equality<UserDataRecord> {
  const UserDataRecordDocumentEquality();

  @override
  bool equals(UserDataRecord? e1, UserDataRecord? e2) {
    return e1?.id == e2?.id &&
        e1?.fName == e2?.fName &&
        e1?.email == e2?.email &&
        e1?.age == e2?.age &&
        e1?.displayName == e2?.displayName &&
        e1?.photoUrl == e2?.photoUrl &&
        e1?.uid == e2?.uid &&
        e1?.createdTime == e2?.createdTime &&
        e1?.phoneNumber == e2?.phoneNumber &&
        e1?.q1 == e2?.q1 &&
        e1?.q2 == e2?.q2 &&
        e1?.q3 == e2?.q3 &&
        e1?.q4 == e2?.q4;
  }

  @override
  int hash(UserDataRecord? e) => const ListEquality().hash([
        e?.id,
        e?.fName,
        e?.email,
        e?.age,
        e?.displayName,
        e?.photoUrl,
        e?.uid,
        e?.createdTime,
        e?.phoneNumber,
        e?.q1,
        e?.q2,
        e?.q3,
        e?.q4
      ]);

  @override
  bool isValidKey(Object? o) => o is UserDataRecord;
}
