import 'dart:convert';

import '/flutter_flow/flutter_flow_util.dart';
import 'api_manager.dart';

export 'api_manager.dart' show ApiCallResponse;

const _kPrivateApiFunctionName = 'ffPrivateApiCall';

class SentimentAPICall {
  static Future<ApiCallResponse> call({
    String? txt = 'so happy',
  }) async {
    return ApiManager.instance.makeApiCall(
      callName: 'Sentiment API',
      apiUrl: 'https://text-sentiment.p.rapidapi.com/analyze',
      callType: ApiCallType.POST,
      headers: {
        'X-RapidAPI-Key': 'c4b5698917mshd39be5e94b9d2afp125f57jsn7d41bfca2dd0',
        'X-RadidAPI-Host': 'text-sentiment.p.rapidapi.com',
        'Content-Type': 'application/x-www-form-urlencoded',
      },
      params: {
        'text': txt,
      },
      bodyType: BodyType.X_WWW_FORM_URL_ENCODED,
      returnBody: true,
      encodeBodyUtf8: false,
      decodeUtf8: false,
      cache: false,
      alwaysAllowBody: false,
    );
  }

  static String? text(dynamic response) => castToType<String>(getJsonField(
        response,
        r'''$.text''',
      ));
  static int? pos(dynamic response) => castToType<int>(getJsonField(
        response,
        r'''$.pos''',
      ));
  static int? neg(dynamic response) => castToType<int>(getJsonField(
        response,
        r'''$.neg''',
      ));
  static int? mid(dynamic response) => castToType<int>(getJsonField(
        response,
        r'''$.mid''',
      ));
}

class AiBotAPICall {
  static Future<ApiCallResponse> call({
    String? quesVal = '',
  }) async {
    final ffApiRequestBody = '''
{
  "model": "gpt-3.5-turbo",
  "messages": [
    {
      "role": "user",
      "content": "$quesVal"
    }
  ]
}''';
    return ApiManager.instance.makeApiCall(
      callName: 'aiBotAPI',
      apiUrl: 'https://api.openai.com/v1/chat/completions',
      callType: ApiCallType.POST,
      headers: {
        'Content-Type': 'application/json',
        'Authorization':
            'Bearer sk-proj-qAQ7VzNQ1K6fmlEWRaDWT3BlbkFJB8R44iGiXBf70mDuKFkj',
      },
      params: {},
      body: ffApiRequestBody,
      bodyType: BodyType.JSON,
      returnBody: true,
      encodeBodyUtf8: false,
      decodeUtf8: false,
      cache: false,
      alwaysAllowBody: false,
    );
  }

  static String? apiResp(dynamic response) => castToType<String>(getJsonField(
        response,
        r'''$.choices[:].message.content''',
      ));
}

class ApiPagingParams {
  int nextPageNumber = 0;
  int numItems = 0;
  dynamic lastResponse;

  ApiPagingParams({
    required this.nextPageNumber,
    required this.numItems,
    required this.lastResponse,
  });

  @override
  String toString() =>
      'PagingParams(nextPageNumber: $nextPageNumber, numItems: $numItems, lastResponse: $lastResponse,)';
}

String _serializeList(List? list) {
  list ??= <String>[];
  try {
    return json.encode(list);
  } catch (_) {
    return '[]';
  }
}

String _serializeJson(dynamic jsonVar, [bool isList = false]) {
  jsonVar ??= (isList ? [] : {});
  try {
    return json.encode(jsonVar);
  } catch (_) {
    return isList ? '[]' : '{}';
  }
}
