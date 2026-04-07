// fast-xml2js.cpp
#include <node.h>
#include "rapidxml/rapidxml_utils.hpp"
#include <stack>
#include <cstring>

namespace fastxml2js {

using v8::Context;
using v8::Function;
using v8::Exception;
using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Number;
using v8::Value;
using v8::Array;
using v8::MaybeLocal;
using v8::Maybe;

using namespace rapidxml;

void ParseString(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if(args.Length() != 2)
  {
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, "Wrong number of arguments").ToLocalChecked()));
    return;
  }

  if(!args[0]->IsString())
  {
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, "First argument must be a string").ToLocalChecked()));
    return;
  }

  if(!args[1]->IsFunction())
  {
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, "Second argument must be a callback").ToLocalChecked()));
    return;
  }

  // String::Utf8Value param1(args[0]->ToString());
  Local<Context> context = isolate->GetCurrentContext();
  String::Utf8Value param1(isolate, args[0]->ToString(context).ToLocalChecked());


  char *xml = new char[param1.length() + 1];
  std::strcpy(xml, *param1);

  Local<Object> obj = Object::New(isolate);
  Local<Value> errorString = Null(isolate);

  try
  {

    xml_document<> doc;
    doc.parse<0>(xml);

    std::stack<xml_node<> *> nodeStack;
    std::stack<Local<Object> > objStack;

    nodeStack.push(doc.first_node());
    objStack.push(obj);

    while(nodeStack.size() > 0)
    {
      xml_node<> *node = nodeStack.top();
      if(!node)
      {
        nodeStack.pop();
        objStack.pop();
        continue;
      }

      Local<Object> obj = objStack.top();

      Local<Object> newObj = Object::New(isolate);

      bool hasChild = false;

      //Need to reduce duplicate code here
      if(!node->first_node() || (node->first_node() && node->first_node()->type() != node_cdata && node->first_node()->type() != node_data))
      {
        hasChild = true;

        Local<Array> lst;
        if(node != doc.first_node())
        {
          Maybe<bool> hasProperty = obj->HasOwnProperty(
              context, 
              String::NewFromUtf8(isolate, node->name()).ToLocalChecked()
          );
          if(hasProperty.FromMaybe(false))
          {
            MaybeLocal<Value> maybeVal = obj->Get(context, String::NewFromUtf8(isolate, node->name()).ToLocalChecked());
            if(maybeVal.IsEmpty()) {
               isolate->ThrowException(Exception::TypeError(
                 String::NewFromUtf8(isolate, "Failed to get property").ToLocalChecked()
               ));
               return;
            }
            lst = Local<Array>::Cast(maybeVal.ToLocalChecked());
            lst->Set(context, String::NewFromUtf8(isolate, "length").ToLocalChecked(), Number::New(isolate, lst->Length() + 1)).ToChecked();
          }
          else
           {
             lst = Array::New(isolate, 1);
             obj->Set(context, String::NewFromUtf8(isolate, node->name()).ToLocalChecked(), lst).ToChecked();
           }

           lst->Set(context, lst->Length()-1, newObj).ToChecked();
         }
         else
         {
           obj->Set(context, String::NewFromUtf8(isolate, node->name()).ToLocalChecked(), newObj).ToChecked();
         }
      }
      else
      {
        Local<Array> lst;
        if(node != doc.first_node())
        {
         if(obj->HasOwnProperty(context, String::NewFromUtf8(isolate, node->name()).ToLocalChecked()).FromMaybe(false))
          {
            lst = Local<Array>::Cast(obj->Get(context, String::NewFromUtf8(isolate, node->name()).ToLocalChecked()).ToLocalChecked());
            lst->Set(context, String::NewFromUtf8(isolate, "length").ToLocalChecked(), Number::New(isolate, lst->Length() + 1)).ToChecked();
          }
          else
          {
            lst = Array::New(isolate, 1);
            obj->Set(context, String::NewFromUtf8(isolate, node->name()).ToLocalChecked(), lst).ToChecked();
          }

         if(node->first_attribute()) {
            Local<Object> attrObj = Object::New(isolate);
            newObj->Set(context, String::NewFromUtf8(isolate, "_").ToLocalChecked(), String::NewFromUtf8(isolate, node->first_node()->value()).ToLocalChecked()).ToChecked();
            newObj->Set(context, String::NewFromUtf8(isolate, "$").ToLocalChecked(), attrObj).ToChecked();

            for(xml_attribute<> *attr = node->first_attribute(); attr; attr = attr->next_attribute())
            {
              attrObj->Set(context, String::NewFromUtf8(isolate, attr->name()).ToLocalChecked(), String::NewFromUtf8(isolate, attr->value()).ToLocalChecked()).ToChecked();
            }

            lst->Set(context, lst->Length()-1, newObj).ToChecked();
          }
          else {
            lst->Set(context, lst->Length()-1, String::NewFromUtf8(isolate, node->first_node()->value()).ToLocalChecked()).ToChecked();
          }
        }
       else
         {
           obj->Set(context, String::NewFromUtf8(isolate, node->name()).ToLocalChecked(), String::NewFromUtf8(isolate, node->first_node()->value()).ToLocalChecked()).ToChecked();
         }
      }

      nodeStack.pop();
      nodeStack.push(node->next_sibling());

      if(hasChild) {

         if(node->first_attribute())
         {
           Local<Object> attrObj = Object::New(isolate);
           newObj->Set(context, String::NewFromUtf8(isolate, "$").ToLocalChecked(), attrObj).ToChecked();

           for(xml_attribute<> *attr = node->first_attribute(); attr; attr = attr->next_attribute())
            {
              attrObj->Set(context, String::NewFromUtf8(isolate, attr->name()).ToLocalChecked(), String::NewFromUtf8(isolate, attr->value()).ToLocalChecked()).ToChecked();
            }
          }

        nodeStack.push(node->first_node());
        objStack.push(newObj);
      }

    }

  }
  catch (const std::runtime_error& e)
  {
    errorString = String::NewFromUtf8(isolate, e.what()).ToLocalChecked();
  }
  catch (const rapidxml::parse_error& e)
  {
    errorString = String::NewFromUtf8(isolate, e.what()).ToLocalChecked();
  }
  catch (const std::exception& e)
  {
    errorString = String::NewFromUtf8(isolate, e.what()).ToLocalChecked();
  }
  catch (const Local<Value>& e)
  {
    errorString = e;
  }
  catch (...)
  {
    errorString = String::NewFromUtf8(isolate, "An unknown error occurred while parsing.").ToLocalChecked();
  }

  delete[] xml;

 Local<Function> cb = Local<Function>::Cast(args[1]);
  const unsigned argc = 2;
  Local<Value> argv[argc] = { errorString, obj };

  MaybeLocal<Value> callResult = cb->Call(context, Null(isolate), argc, argv);
  if(callResult.IsEmpty()) {
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, "Callback invocation failed").ToLocalChecked()
    ));
  }
}

void init(Local<Object> exports) {
  NODE_SET_METHOD(exports, "parseString", ParseString);
}

#ifdef _WIN32
#pragma warning(disable : 4244 4267)
#endif

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif

NODE_MODULE_X(fastxml2js, init, 0, 0)

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

}  // namespace fastxml2js


