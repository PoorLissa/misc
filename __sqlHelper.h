#pragma once
#ifndef SQL_HELPER_H
#define SQL_HELPER_H

/*
    Usage:
 ------------------------------------------------------------------------------

    using sqlHelper::sql;

    sql _sql;

    _sql.SELECT("obj_name1, obj_name2")
        .FROM("table_name")
            .LEFT_JOIN("other_table_name")
                .ON("param = 1")
        .WHERE("id = 2")
            .AND("calc1 = 3")
                .AND("calc2 = 4")
        .ORDER_BY("id");

    AnsiString str = _sql;

    ==> str = " SELECT obj_name1, obj_name2 FROM table_name LEFT JOIN other_table_name ON param = 1 WHERE id = 2 AND calc1 = 3 AND calc2 = 4 ORDER BY id"
 ------------------------------------------------------------------------------
*/

namespace sqlHelper
{
    #define  sql       qHelper
    #define  Str       AnsiString
    #define cStr const AnsiString
    #define cInt const int

    class qHelper;
    class qSelect;
    class qUpdate;
    class qFrom;
    class qWhere;
    class qLeftJoin;
    class qOn;
    class qAnd;
    class qOrderBy;
    class qGroupBy;
    class qSet;
    class qValues;

    static AnsiString Command[] = { "",   "SELECT ", " FROM ", " WHERE ", " LEFT JOIN ", " ORDER BY ", " ON ", " AND ", " GROUP BY ", "UPDATE ", " SET ", "DELETE FROM ", "INSERT INTO ", " VALUES ( " };
    enum   CommandsEnum           { Empty, Select,     From,     Where,     LeftJoin,      OrderBy,      On,     And,     GroupBy,     Update,     Set,    DeleteFrom,     InsertInto,      Values     };



    class qCommandBase {
        qCommandBase& operator =(qCommandBase &);
        
     public:
        qCommandBase(qHelper *query, const AnsiString str) : _query(query), _str(str) {}
        virtual ~qCommandBase() = 0;

        // Перегружаем оператор преобразования к AnsiString, чтобы не заморачиваться с вызовом метода get(),
        // и чтобы каждый объект мог вернуть итоговую строку запроса
        inline operator AnsiString() const { return _str; }

     protected:

        // Главный метод. Достраивает строку, сохраняет ее и возвращает объект нужного типа.
        template<class Type>
        Type QueryFunc(qHelper *Query, const AnsiString Data, const AnsiString Command)
        {
            Query->set(Query->get() + Command + Data);
            return Type(Query, Query->get());
        }

        qHelper    *_query;
        AnsiString  _str;
    };

    qCommandBase::~qCommandBase()
    {
    }



    // Вынес повторяющееся семейство методов WHERE в отдельный класс
    class WhereFamily : public qCommandBase {
        WhereFamily& operator =(WhereFamily &);
     public:
        WhereFamily(qHelper *query, const AnsiString str) : qCommandBase(query, str) {}

        qWhere WHERE (cStr str       ) { return QueryFunc<qWhere>(_query, str,                         Command[Where]); }
        qWhere WHERE (Str str, cInt i) { return QueryFunc<qWhere>(_query, str.sprintf(str.c_str(), i), Command[Where]); }   // %i
        qWhere WHERE (Str str, cStr s) { return QueryFunc<qWhere>(_query, str.sprintf(str.c_str(), s), Command[Where]); }   // %s
    };



    // Вынес повторяющееся семейство методов AND в отдельный класс
    class AndFamily : public qCommandBase {
        AndFamily& operator =(AndFamily &);
     public:
        AndFamily(qHelper *query, const AnsiString str) : qCommandBase(query, str) {}

        qAnd AND (const AnsiString str) { return QueryFunc<qAnd>(_query, str,                         Command[And]); }
        qAnd AND (Str str, cInt i     ) { return QueryFunc<qAnd>(_query, str.sprintf(str.c_str(), i), Command[And]); }      // %i
        qAnd AND (Str str, cStr s     ) { return QueryFunc<qAnd>(_query, str.sprintf(str.c_str(), s), Command[And]); }      // %s
    };



    class qSelect : public qCommandBase {
        qSelect& operator =(qSelect &);
     public:
        qSelect(qHelper *query, const AnsiString str) : qCommandBase(query, str) {}

        qFrom FROM(const AnsiString str) { return QueryFunc<qFrom>(_query, str, Command[From]); }

    };



    class qUpdate : public qCommandBase {
        qUpdate& operator =(qUpdate &);
     public:
        qUpdate(qHelper *query, const AnsiString str) : qCommandBase(query, str) {}

        qSet SET(const AnsiString str) { return QueryFunc<qSet>(_query, str,                         Command[Set]); }
        qSet SET(Str str, cInt n     ) { return QueryFunc<qSet>(_query, str.sprintf(str.c_str(), n), Command[Set]); }
        qSet SET(Str str, cStr s     ) { return QueryFunc<qSet>(_query, str.sprintf(str.c_str(), s), Command[Set]); }
    };



    class qDeleteFrom : public WhereFamily {
        qDeleteFrom& operator =(qDeleteFrom &);
     public:
        qDeleteFrom(qHelper *query, const AnsiString str) : WhereFamily(query, str) {}
    };



    class qInsertInto : public qCommandBase {
        qInsertInto& operator =(qInsertInto &);
     public:
        qInsertInto(qHelper *query, const AnsiString str) : qCommandBase(query, str) {}

        qValues VALUES(const AnsiString str) { return QueryFunc<qValues>(_query, str, Command[Values]); }
    };



    class qValues : public qCommandBase {
        qValues& operator =(qValues &);
     public:
        qValues(qHelper *query, const AnsiString str) : qCommandBase(query, str) {}

        operator AnsiString () { return _str + " )"; }        
    };



    class qFrom : public WhereFamily {
        qFrom& operator =(qFrom &);
     public:
        qFrom(qHelper *query, const AnsiString str) : WhereFamily(query, str) {}

        qLeftJoin LEFT_JOIN (const AnsiString str) { return QueryFunc<qLeftJoin>(_query, str, Command[LeftJoin]); }
        qOrderBy  ORDER_BY  (const AnsiString str) { return QueryFunc<qOrderBy >(_query, str, Command[OrderBy ]); }
        qGroupBy  GROUP_BY  (const AnsiString str) { return QueryFunc<qGroupBy >(_query, str, Command[GroupBy ]); }
    };



    class qSet : public WhereFamily {
        qSet& operator =(qSet &);
     public:
        qSet(qHelper *query, const AnsiString str) : WhereFamily(query, str) {}
    };



    class qLeftJoin : public WhereFamily {
        qLeftJoin& operator =(qLeftJoin &);
     public:
        qLeftJoin(qHelper *query, const AnsiString str) : WhereFamily(query, str) {}

        qOn      ON       (const AnsiString str) { return QueryFunc<qOn     >(_query, str, Command[On     ]); }
        qOrderBy ORDER_BY (const AnsiString str) { return QueryFunc<qOrderBy>(_query, str, Command[OrderBy]); }
    };



    class qOn : public WhereFamily {
        qOn& operator =(qOn &);
     public:
        qOn(qHelper *query, const AnsiString str) : WhereFamily(query, str) {}

        qOrderBy ORDER_BY (const AnsiString str) { return QueryFunc<qOrderBy>(_query, str, Command[OrderBy]); }
    };



    class qWhere : public AndFamily {
        qWhere& operator =(qWhere &);
     public:
        qWhere(qHelper *query, const AnsiString str) : AndFamily(query, str) {}

        qOrderBy ORDER_BY (const AnsiString str) { return QueryFunc<qOrderBy>(_query, str, Command[OrderBy]); }
    };



    class qAnd : public AndFamily {
        qAnd& operator =(qAnd &);
     public:
        qAnd(qHelper *query, const AnsiString str) : AndFamily(query, str) {}

        qOrderBy ORDER_BY (const AnsiString str) { return QueryFunc<qOrderBy>(_query, str, Command[OrderBy]); }
    };



    class qOrderBy : public qCommandBase {
        qOrderBy& operator =(qOrderBy &);
     public:
        qOrderBy(qHelper *query, const AnsiString str) : qCommandBase(query, str) {}
    };



    class qGroupBy : public qCommandBase {
        qGroupBy& operator =(qGroupBy &);
     public:
        qGroupBy(qHelper *query, const AnsiString str) : qCommandBase(query, str) {}

        qOrderBy ORDER_BY (const AnsiString str) { return QueryFunc<qOrderBy>(_query, str, Command[OrderBy]); }        
    };



    // Основной класс, через объект которого будем создавать запрос
    class qHelper : public qCommandBase {
        qHelper& operator =(qHelper &);    
     public:
        qHelper() : qCommandBase(this, Command[Empty]) {}

        // При вызове SELECT или UPDATE начинаем собирать запрос с пустой строки
        qSelect     SELECT      (const AnsiString str) { _str = Command[Empty]; return QueryFunc<qSelect    >(this, str, Command[Select    ]); }
        qUpdate     UPDATE      (const AnsiString str) { _str = Command[Empty]; return QueryFunc<qUpdate    >(this, str, Command[Update    ]); }
        qDeleteFrom DELETE_FROM (const AnsiString str) { _str = Command[Empty]; return QueryFunc<qDeleteFrom>(this, str, Command[DeleteFrom]); }
        qInsertInto INSERT_INTO (const AnsiString str) { _str = Command[Empty]; return QueryFunc<qInsertInto>(this, str, Command[InsertInto]); }

        inline void       set(AnsiString str) {  _str = str; }
        inline AnsiString get() const         { return _str; }
    };
}

#endif
