#pragma once
#ifndef SQL_HELPER_H
#define SQL_HELPER_H

/*
    Usage:
 ------------------------------------------------------------------------------

    using sqlHelper::sql;

    sql _sql, qh1;

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

    qh1.SELECT(

        qh1.lB().SELECT("asd").FROM("asd").WHERE("asd").rB() +
        qh1.SELECT("*").FROM("tab1").WHERE("where").AND("asdasd") +
        qh1.SELECT("*").FROM("tab2").WHERE("asd") +
        qh1.txt(" preved medved ") +
        qh1.lB().SELECT("imu_adr").FROM("BI_R_mzayav" ).WHERE("calc_id=%i", 170520).rB().AS("t1")
      + qh1.lB().SELECT("prem"   ).FROM("BI_R_objects").WHERE("calc_id=%i", 170523).AND("num=%i", 6).rB().AS("t2")
      + "StrSumm"

    )
    .FROM("BI_R_main").WHERE("calc_id=%i", 170523);

    == > SELECT (SELECT * FROM (tab1) WHERE id=1), SELECT * FROM (tab2) WHERE id=2,  preved medved , (SELECT imu_adr FROM (BI_R_mzayav) WHERE calc_id=170520) AS t1, (SELECT prem FROM (BI_R_objects) WHERE calc_id=170523 AND num=6) AS t2, StrSumm FROM (BI_R_main) WHERE calc_id=170523

 ------------------------------------------------------------------------------
*/

namespace sqlHelper
{
    #define  sql       qHelper
    #define  Str       AnsiString
    #define cStr const AnsiString
    #define cInt const int

    class qHelper;
    class qAny;
    class qSelect;
    class qUpdate;
    class qFrom;
    class qWhere;
    class qLeftJoin;
    class qRightJoin;    
    class qOn;
    class qAnd;
    class qOrderBy;
    class qGroupBy;
    class qSet;
    class qValues;
    class qAs;
    class qLBracket;
    class qRBracket;

    static AnsiString Command[] = { "",   "(",  ")", "SELECT ", " FROM ", " WHERE ", " LEFT JOIN ", " RIGHT JOIN ", " ORDER BY ", " ON ", " AND ", " GROUP BY ", "UPDATE ", " SET ", "DELETE FROM ", "INSERT INTO ", " VALUES ( ", " AS " };
    enum   CommandsEnum           { Empty, lBr, rBr, Select,     From,     Where,     LeftJoin,      RightJoin,      OrderBy,      On,     And,     GroupBy,     Update,     Set,    DeleteFrom,     InsertInto,      Values    ,   As   };

    

    class qCommandBase {
        qCommandBase& operator =(qCommandBase &);

     public:
        qCommandBase(qHelper *query, const AnsiString str) : _query(query), _str(str) {}
        virtual ~qCommandBase() = 0;

        // Перегружаем оператор преобразования к AnsiString, чтобы не заморачиваться с вызовом метода get(),
        // и чтобы каждый объект мог вернуть итоговую строку запроса
        inline operator AnsiString() const { return _str; }

        // --- Общие методы для всех субклассов ---

        // Добавляем произвольный текст
        qAny txt(const AnsiString str) { _str = Command[Empty]; return QueryFunc<qAny>(_query, str, Empty); }

     protected:

        // Главный метод. Достраивает строку, сохраняет ее и возвращает объект нужного типа.
        template<class Type>
        Type QueryFunc(qHelper *Query, const AnsiString Data, const int CommandNo)
        {
//            return sqlHelper::qqq<Type>(Query, Data, Command);

            AnsiString *command = &sqlHelper::Command[CommandNo];

            if( CommandNo == From )
            {
                Query->set(Query->get() + *command + "(" + Data + ")");
            }
            else
            {
                Query->set(Query->get() + *command + Data);
            }

            return Type(Query, Query->get());
        }

     protected:
        qHelper    *_query;
        AnsiString  _str;
    };

    qCommandBase::~qCommandBase()
    {
    }



    // Промежуточный класс для суммирования объектов (через запятую или через пробел)
    class qSummable : public qCommandBase {
        qSummable& operator =(qSummable &);

     public:
        qSummable(qHelper *query, const AnsiString str) : qCommandBase(query, str) {}

        friend qSummable operator +(const char *, qSummable);
        friend qSummable operator &(const char *, qSummable);

        qSummable operator +(qSummable other)
        {
            sum_viaComma(other._str);
            return *this;
        }

        qSummable operator +(AnsiString other)
        {
            sum_viaComma(other);
            return *this;
        }

        qSummable operator &(qSummable other)
        {
            sum_viaSpace(other._str);
            return *this;
        }

        qSummable operator &(AnsiString other)
        {
            sum_viaSpace(other);
            return *this;
        }

     private:
        inline void sum_viaComma(AnsiString &str)
        {
            _str += ", " + str;
        }

        inline void sum_viaSpace(AnsiString &str)
        {
            _str += " " + str;
        }
    };

    qSummable operator +(const char *str, qSummable other)
    {
        other._str = AnsiString(str) + ", " + other._str;
        return other;
    }

    qSummable operator &(const char *str, qSummable other)
    {
        other._str = AnsiString(str) + " " + other._str;
        return other;
    }



    // Вынес повторяющееся семейство методов WHERE в отдельный класс
    class WhereFamily : public qSummable {
        WhereFamily& operator =(WhereFamily &);
     public:
        WhereFamily(qHelper *query, const AnsiString str) : qSummable(query, str) {}

        qWhere WHERE (cStr str       ) { return QueryFunc<qWhere>(_query, str,                         Where); }
        qWhere WHERE (Str str, cInt i) { return QueryFunc<qWhere>(_query, str.sprintf(str.c_str(), i), Where); }   // %i
        qWhere WHERE (Str str, cStr s) { return QueryFunc<qWhere>(_query, str.sprintf(str.c_str(), s), Where); }   // %s
    };



    // Вынес повторяющееся семейство методов AND в отдельный класс
    class AndFamily : public qSummable {
        AndFamily& operator =(AndFamily &);
     public:
        AndFamily(qHelper *query, const AnsiString str) : qSummable(query, str) {}

        qAnd AND (const AnsiString str) { return QueryFunc<qAnd>(_query, str,                         And); }
        qAnd AND (Str str, cInt i     ) { return QueryFunc<qAnd>(_query, str.sprintf(str.c_str(), i), And); }      // %i
        qAnd AND (Str str, cStr s     ) { return QueryFunc<qAnd>(_query, str.sprintf(str.c_str(), s), And); }      // %s
    };



    class qSelect : public qCommandBase {
        qSelect& operator =(qSelect &);
     public:
        qSelect(qHelper *query, const AnsiString str) : qCommandBase(query, str) {}

        qFrom FROM (const AnsiString str) { return QueryFunc<qFrom>(_query, str, From ); }
        qAs   AS   (const AnsiString str) { return QueryFunc<qAs  >(_query, str, As   ); }
    };



    class qAny : public qSummable {
        qAny& operator =(qAny &);
     public:
        qAny(qHelper *query, const AnsiString str) : qSummable(query, str) {}

        // Добавить методы по мере необходимости
        qSelect SELECT  (const AnsiString str) { return QueryFunc<qSelect>(_query, str, Select); }
        qFrom   FROM    (const AnsiString str) { return QueryFunc<qFrom  >(_query, str, From  ); }
        qAs     AS      (const AnsiString str) { return QueryFunc<qAs    >(_query, str, As    ); }
    };



    class qAs : public qSummable {
        qAs& operator =(qAs &);
     public:
        qAs(qHelper *query, const AnsiString str) : qSummable(query, str) {}

        qFrom FROM (const AnsiString str) { return QueryFunc<qFrom>(_query, str, From); }
        qAs   AS   (const AnsiString str) { return QueryFunc<qAs  >(_query, str, As  ); }
    };



    class qUpdate : public qCommandBase {
        qUpdate& operator =(qUpdate &);
     public:
        qUpdate(qHelper *query, const AnsiString str) : qCommandBase(query, str) {}

        qSet SET(const AnsiString str) { return QueryFunc<qSet>(_query, str,                         Set); }
        qSet SET(Str str, cInt n     ) { return QueryFunc<qSet>(_query, str.sprintf(str.c_str(), n), Set); }
        qSet SET(Str str, cStr s     ) { return QueryFunc<qSet>(_query, str.sprintf(str.c_str(), s), Set); }
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

        qValues VALUES(const AnsiString str) { return QueryFunc<qValues>(_query, str, Values); }
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

        qLeftJoin  LEFT_JOIN  (const AnsiString str) { return QueryFunc<qLeftJoin >(_query, str, LeftJoin ); }
        qRightJoin RIGHT_JOIN (const AnsiString str) { return QueryFunc<qRightJoin>(_query, str, RightJoin); }
        qOrderBy   ORDER_BY   (const AnsiString str) { return QueryFunc<qOrderBy  >(_query, str, OrderBy  ); }
        qGroupBy   GROUP_BY   (const AnsiString str) { return QueryFunc<qGroupBy  >(_query, str, GroupBy  ); }

        qFrom operator [] (AnsiString str)
        {
            return QueryFunc<qFrom>(_query, str, From);
        }
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

        qOn      ON       (const AnsiString str) { return QueryFunc<qOn     >(_query, str, On     ); }
        qOrderBy ORDER_BY (const AnsiString str) { return QueryFunc<qOrderBy>(_query, str, OrderBy); }
    };



    class qRightJoin : public WhereFamily {
        qRightJoin& operator =(qRightJoin &);
     public:
        qRightJoin(qHelper *query, const AnsiString str) : WhereFamily(query, str) {}

        qOn      ON       (const AnsiString str) { return QueryFunc<qOn     >(_query, str, On     ); }
        qOrderBy ORDER_BY (const AnsiString str) { return QueryFunc<qOrderBy>(_query, str, OrderBy); }
    };



    class qOn : public WhereFamily {
        qOn& operator =(qOn &);
     public:
        qOn(qHelper *query, const AnsiString str) : WhereFamily(query, str) {}

        qOrderBy ORDER_BY (const AnsiString str) { return QueryFunc<qOrderBy>(_query, str, OrderBy); }
    };



    class qWhere : public AndFamily {
        qWhere& operator =(qWhere &);
     public:
        qWhere(qHelper *query, const AnsiString str) : AndFamily(query, str) {}

        qOrderBy  ORDER_BY (const AnsiString str) {                        return QueryFunc<qOrderBy >(_query, str, OrderBy); }
        qRBracket rB       ()                     { _str = Command[Empty]; return QueryFunc<qRBracket>(_query, _str, rBr   ); }
    };



    class qAnd : public AndFamily {
        qAnd& operator =(qAnd &);
     public:
        qAnd(qHelper *query, const AnsiString str) : AndFamily(query, str) {}

        qOrderBy  ORDER_BY (const AnsiString str) {                        return QueryFunc<qOrderBy >(_query, str, OrderBy); }
        qRBracket rB       ()                     { _str = Command[Empty]; return QueryFunc<qRBracket>(_query, _str, rBr   ); }
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

        qOrderBy ORDER_BY (const AnsiString str) { return QueryFunc<qOrderBy>(_query, str, OrderBy); }
    };



    class qLBracket : public qSummable {
        qLBracket& operator =(qLBracket &);
     public:
        qLBracket(qHelper *query, const AnsiString str) : qSummable(query, str) {}

        qSelect     SELECT      (const AnsiString str) { _str = Command[Empty]; return QueryFunc<qSelect    >(_query, str, Select    ); }
        qUpdate     UPDATE      (const AnsiString str) { _str = Command[Empty]; return QueryFunc<qUpdate    >(_query, str, Update    ); }
        qDeleteFrom DELETE_FROM (const AnsiString str) { _str = Command[Empty]; return QueryFunc<qDeleteFrom>(_query, str, DeleteFrom); }
        qInsertInto INSERT_INTO (const AnsiString str) { _str = Command[Empty]; return QueryFunc<qInsertInto>(_query, str, InsertInto); }
    };



    class qRBracket : public qSummable {
        qRBracket& operator =(qRBracket &);
     public:
        qRBracket(qHelper *query, const AnsiString str) : qSummable(query, str) {}

        qAs   AS   (const AnsiString str) { return QueryFunc<qAs  >(_query, str, As   ); }
    };

    // ------------------------------------------------------------------------

    // Основной класс, через объект которого будем создавать запрос
    class qHelper : public qCommandBase {
        qHelper& operator =(qHelper &);
     public:
        qHelper() : qCommandBase(this, Command[Empty]) {}

        // При вызове SELECT или UPDATE начинаем собирать запрос с пустой строки
        qSelect     SELECT      (const AnsiString str) { _str = Command[Empty]; return QueryFunc<qSelect    >(this, str, Select    ); }
        qUpdate     UPDATE      (const AnsiString str) { _str = Command[Empty]; return QueryFunc<qUpdate    >(this, str, Update    ); }
        qDeleteFrom DELETE_FROM (const AnsiString str) { _str = Command[Empty]; return QueryFunc<qDeleteFrom>(this, str, DeleteFrom); }
        qInsertInto INSERT_INTO (const AnsiString str) { _str = Command[Empty]; return QueryFunc<qInsertInto>(this, str, InsertInto); }
        qLBracket   lB          ()                     { _str = Command[Empty]; return QueryFunc<qLBracket  >(this,_str, lBr       ); }

        inline void       set   (AnsiString str)       {  _str = str; }
        inline AnsiString get   () const               { return _str; }
    };


    // ------------------------------------------------------------------------


    template <class Type>
    Type qqq(qHelper *Query, const AnsiString Data, const AnsiString Command)
    {
        Query->set(Query->get() + Command + Data);
        return Type(Query, Query->get());
    }

    template <>
    qFrom qqq(qHelper *Query, const AnsiString Data, const AnsiString Command)
    {
        Query->set(Query->get() + "(" + Command + Data + ")");
        return qFrom(Query, Query->get());
    }


    // ------------------------------------------------------------------------
}

#endif

